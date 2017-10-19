// For license of this file, see <object-root-folder>/LICENSE.md.

#include "core/messagesmodelcache.h"

#include "miscellaneous/textfactory.h"

MessagesModelCache::MessagesModelCache(QObject* parent) : QObject(parent), m_msgCache(QHash<int, QSqlRecord>()) {}

MessagesModelCache::~MessagesModelCache() {}

void MessagesModelCache::setData(const QModelIndex& index, const QVariant& value, const QSqlRecord& record) {
  if (!m_msgCache.contains(index.row())) {
    m_msgCache[index.row()] = record;
  }

  m_msgCache[index.row()].setValue(index.column(), value);
}

QVariant MessagesModelCache::data(const QModelIndex& idx) {
  return m_msgCache[idx.row()].value(idx.column());
}
