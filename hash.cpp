#include "hash.h"
#include "table.h"

Hash::Hash(Database const* db, std::string const& name, bool isUnique,
      std::vector<std::pair<Column, Direction>> const& cols):
   Index(db, name, isUnique, Index::Hash, cols) {
   m_recordSize = sizeof(uint8_t) + sizeof(rowcount_t);
   for(auto it = m_cols.begin(); it != m_cols.end(); ++it)
      m_recordSize += it->first.size();
}

Hash::Hash(Database const* db, std::string const& name): Index(db, name) {
   m_recordSize = sizeof(uint8_t) + sizeof(rowcount_t);
   for(auto it = m_cols.begin(); it != m_cols.end(); ++it)
      m_recordSize += it->first.size();
}

// TODO: implement
bool Hash::remove(rowiterator const& rowIt) const {
   return true;
}

// TODO: implement
bool Hash::insert(rowcount_t row, std::map<Column, void*> const& values) const {
   return true;
}

// TODO: implement
rowiterator Hash::rowIterator(std::map<Column,std::vector<Predicate>> const& preds) const {

}
