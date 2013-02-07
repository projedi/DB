#include "hash.h"
#include "table.h"

#include <iostream>
using namespace std;

pagesize_t const HASH_SIZE = 1000000;

Hash::Hash(Database const* db, std::string const& name, bool isUnique,
      std::vector<std::pair<Column, Direction>> const& cols):
   Index(db, name, isUnique, Index::Hash, cols) {
   m_recordSize = sizeof(uint8_t) + sizeof(rowcount_t);
   for(auto it = m_cols.begin(); it != m_cols.end(); ++it)
      m_recordSize += it->first.size();
   m_hashSize = HASH_SIZE;
}

Hash::Hash(Database const* db, std::string const& name): Index(db, name) {
   m_recordSize = sizeof(uint8_t) + sizeof(rowcount_t);
   for(auto it = m_cols.begin(); it != m_cols.end(); ++it)
      m_recordSize += it->first.size();
   m_hashSize = HASH_SIZE;
}


// TODO: implement
bool Hash::remove(rowiterator const& rowIt) const {
   return true;
}

bool Hash::insert(rowcount_t row, std::map<Column, void*> const& values) const {
   auto colIt = m_cols.begin();
   auto hash = colIt->first.type().hash(values.at(colIt->first), m_hashSize);
   ++colIt;
   for(;colIt != m_cols.end(); ++colIt) 
      hash ^= colIt->first.type().hash(values.at(colIt->first), m_hashSize);
   cerr << "Inserting with hash = " << hash << endl;
   std::unique_ptr<Page> page;
   pagesize_t pageSize = m_page.db()->metadata().pageSize;
   pagesize_t recordsOnFirstPage = (pageSize - m_headerSize) / m_recordSize; 
   pagesize_t recordsOnPage = pageSize / m_recordSize;
   pagesize_t pageOffset;
   if(hash < recordsOnFirstPage) {
      pageOffset = m_headerSize + hash * m_recordSize;
      page.reset(new Page(m_page));
   } else {
      hash -= recordsOnFirstPage;
      pagenumber_t pageNum = hash / recordsOnPage + 1; 
      page.reset(new Page(m_page.db(), m_page.name(), pageNum));
      pageOffset = (hash - (pageNum - 1) * recordsOnPage) * m_recordSize;
   }
   while(true) {
      for(; pageOffset + m_recordSize <= pageSize; pageOffset += m_recordSize) {
         auto offset = pageOffset;
         auto status = page->at<uint8_t>(offset);
         if(status != 0xaa) {
            offset = pageOffset;
            page->at<uint8_t>(offset, 0xaa);
            colIt = m_cols.begin();
            for(; colIt != m_cols.end(); ++colIt) { 
               colIt->first.type().write(values.at(colIt->first), *page, offset);
            }
            page->at<rowcount_t>(offset, row);
            return true;
         }
         if(m_unique) {
            colIt = m_cols.begin();
            std::vector<Predicate> predicate(1, Predicate(Predicate::EQ, (void*)nullptr));
            for(; colIt != m_cols.end(); ++colIt) {
               predicate[0].val = values.at(colIt->first); 
               if(!colIt->first.type().satisfies(predicate, *page, offset)) break;
            }
            if(colIt == m_cols.end()) return false;
         }
      }
      page.reset(new Page(page->db(), page->name(), page->number() + 1));
      pageOffset = 0;
   }
}

// TODO: implement
rowiterator Hash::rowIterator(std::map<Column,std::vector<Predicate>> const& preds) const {

}
