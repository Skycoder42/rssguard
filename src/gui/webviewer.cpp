// For license of this file, see <object-root-folder>/LICENSE.md.

#include "gui/webviewer.h"

#include "definitions/definitions.h"
#include "gui/dialogs/formmain.h"
#include "gui/tabwidget.h"
#include "gui/webbrowser.h"
#include "miscellaneous/application.h"
#include "miscellaneous/skinfactory.h"
#include "network-web/adblock/adblockicon.h"
#include "network-web/adblock/adblockmanager.h"
#include "network-web/webfactory.h"
#include "network-web/webpage.h"

#include <QWheelEvent>

WebViewer::WebViewer(QWidget* parent) : QWebEngineView(parent) {
  WebPage* page = new WebPage(this);

  connect(page, &WebPage::messageStatusChangeRequested, this, &WebViewer::messageStatusChangeRequested);
  setPage(page);
}

bool WebViewer::canIncreaseZoom() {
  return zoomFactor() <= MAX_ZOOM_FACTOR - ZOOM_FACTOR_STEP;
}

bool WebViewer::canDecreaseZoom() {
  return zoomFactor() >= MIN_ZOOM_FACTOR + ZOOM_FACTOR_STEP;
}

WebPage* WebViewer::page() const {
  return qobject_cast<WebPage*>(QWebEngineView::page());
}

void WebViewer::displayMessage() {
  setHtml(m_messageContents, QUrl::fromUserInput(INTERNAL_URL_MESSAGE));
}

bool WebViewer::increaseWebPageZoom() {
  if (canIncreaseZoom()) {
    setZoomFactor(zoomFactor() + ZOOM_FACTOR_STEP);
    return true;
  }
  else {
    return false;
  }
}

bool WebViewer::decreaseWebPageZoom() {
  if (canDecreaseZoom()) {
    setZoomFactor(zoomFactor() - ZOOM_FACTOR_STEP);
    return true;
  }
  else {
    return false;
  }
}

bool WebViewer::resetWebPageZoom() {
  const qreal new_factor = 1.0;

  if (new_factor != zoomFactor()) {
    setZoomFactor(new_factor);
    return true;
  }
  else {
    return false;
  }
}

void WebViewer::loadMessages(const QList<Message>& messages) {
  Skin skin = qApp->skins()->currentSkin();
  QString messages_layout;
  QString single_message_layout = skin.m_layoutMarkup;

  foreach (const Message& message, messages) {
    QString enclosures;
    QString enclosure_images;

    foreach (const Enclosure& enclosure, message.m_enclosures) {
      enclosures += skin.m_enclosureMarkup.arg(enclosure.m_url, tr("Attachment"), enclosure.m_mimeType);

      if (enclosure.m_mimeType.startsWith(QSL("image/"))) {
        // Add thumbnail image.
        enclosure_images += skin.m_enclosureImageMarkup.arg(
          enclosure.m_url,
          enclosure.m_mimeType,
          qApp->settings()->value(GROUP(Messages), SETTING(Messages::MessageHeadImageHeight)).toString());
      }
    }

    messages_layout.append(single_message_layout
                           .arg(message.m_title,
                                tr("Written by ") + (message.m_author.isEmpty() ?
                                                     tr("unknown author") :
                                                     message.m_author),
                                message.m_url,
                                message.m_contents,
                                message.m_created.toString(Qt::DefaultLocaleShortDate),
                                enclosures,
                                message.m_isRead ? "mark-unread" : "mark-read",
                                message.m_isImportant ? "mark-unstarred" : "mark-starred",
                                QString::number(message.m_id))
                           .arg(enclosure_images));
  }

  m_messageContents = skin.m_layoutMarkupWrapper.arg(messages.size() == 1 ? messages.at(0).m_title : tr("Newspaper view"),
                                                     messages_layout);
  bool previously_enabled = isEnabled();

  setEnabled(false);
  displayMessage();
  setEnabled(previously_enabled);
}

void WebViewer::loadMessage(const Message& message) {
  loadMessages(QList<Message>() << message);
}

void WebViewer::clear() {
  bool previously_enabled = isEnabled();

  setEnabled(false);
  setHtml("<!DOCTYPE html><html><body</body></html>", QUrl(INTERNAL_URL_BLANK));
  setEnabled(previously_enabled);
}

void WebViewer::contextMenuEvent(QContextMenuEvent* event) {
  event->accept();
  QMenu* menu = page()->createStandardContextMenu();

  menu->addAction(AdBlockManager::instance()->adBlockIcon());
  menu->addAction(qApp->web()->engineSettingsAction());
  const QPoint pos = event->globalPos();
  QPoint p(pos.x(), pos.y() + 1);

  menu->popup(p);
}

QWebEngineView* WebViewer::createWindow(QWebEnginePage::WebWindowType type) {
  Q_UNUSED(type)
  int index = qApp->mainForm()->tabWidget()->addBrowser(false, false);

  if (index >= 0) {
    return qApp->mainForm()->tabWidget()->widget(index)->webBrowser()->viewer();
  }
  else {
    return nullptr;
  }
}

void WebViewer::wheelEvent(QWheelEvent* event) {
  QWebEngineView::wheelEvent(event);

  if ((event->modifiers() & Qt::ControlModifier) > 0) {
    if (event->delta() > 0) {
      increaseWebPageZoom();
    }
    else if (event->delta() < 0) {
      decreaseWebPageZoom();
    }
  }
}
