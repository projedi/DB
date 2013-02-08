#include "hash.h"
#include "table.h"

//#include <iostream>
//using namespace std;

using std::make_pair;

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


bool Hash::remove(Table const& table, rowiterator const& rowIt) const {
   if(rowIt.owner() == this) {
      pagesize_t offset = rowIt.offset();
      pagenumber_t pageNum = offset / m_page.db()->metadata().pageSize;
      if(!pageNum) {
         m_page.at<uint8_t>(offset, 0xee);
      } else {
         Page page(m_page.db(), m_page.name(), pageNum);
         offset -= pageNum * m_page.db()->metadata().pageSize;
         page.at<uint8_t>(offset, 0xee);
      }
      return true;
   }
   else {
      pagesize_t offset;
      std::shared_ptr<Page> page(table.getPage(rowIt.row(),offset));
      std::map<Column,std::vector<Predicate>> preds;
      page->at<uint8_t>(offset);
      for(auto it = table.cols().begin(); it != table.cols().end(); ++it) {
         preds[*it].push_back(Predicate(Predicate::EQ, it->type().read(*page,offset)));
      }
      auto rowIter = rowIterator(preds);
      bool res = rowIter;
      for(;rowIter;++rowIter) {
         remove(table,rowIter);
      }
      for(auto it = preds.begin(); it != preds.end(); ++it) {
         it->first.type().clear(it->second[0].val);
      }
      // TODO: S&D
      return res;
   }
}

bool Hash::insert(rowcount_t row, std::map<Column, void*> const& values) const {
   auto colIt = m_cols.begin();
   auto hash = colIt->first.type().hash(values.at(colIt->first), m_hashSize);
   ++colIt;
   for(;colIt != m_cols.end(); ++colIt) 
      hash ^= colIt->first.type().hash(values.at(colIt->first), m_hashSize);
   //cout << "Inserting with hash = " << hash << endl;
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

struct HashIterator {
   HashIterator(Page const& page, pagesize_t pageOffset, std::map<Column,std::vector<Predicate>> const& preds, std::vector<std::pair<Column, Index::Direction>> const& cols, pagesize_t recordSize):
   m_page(page), m_pageOffset(pageOffset), m_preds(preds), m_cols(cols), m_recordSize(recordSize) { }

   boost::optional<std::pair<rowcount_t,pagesize_t>> operator()(rowiterator const&) {
      //cout << "Hash iterator called" << endl;
      while(true) {
         for(; m_pageOffset + m_recordSize <= m_page.db()->metadata().pageSize; m_pageOffset += m_recordSize) {
            auto offset = m_pageOffset;
            auto status = m_page.at<uint8_t>(offset);
            if(status == 0xee) continue;
            if(status != 0xaa) return boost::optional<std::pair<rowcount_t,pagesize_t>>();
            auto colIt = m_cols.begin();
            for(; colIt != m_cols.end(); ++colIt) {
               if(!colIt->first.type().satisfies(m_preds.at(colIt->first), m_page, offset)) break;
            }
            if(colIt == m_cols.end()) {
               auto row = m_page.at<rowcount_t>(offset);
               auto res = boost::optional<std::pair<rowcount_t,pagesize_t>>(make_pair(row,m_pageOffset + m_page.number() * m_page.db()->metadata().pageSize));
               m_pageOffset = offset;
               return res;
            }
         }
         m_page = Page(m_page.db(), m_page.name(), m_page.number() + 1);
         m_pageOffset = 0;
      }
   }

   Page m_page;
   pagesize_t m_pageOffset;
   std::map<Column,std::vector<Predicate>> m_preds;
   std::vector<std::pair<Column, Index::Direction>> m_cols;
   pagesize_t m_recordSize;
};

// TODO: implement
rowiterator Hash::rowIterator(std::map<Column,std::vector<Predicate>> const& preds) const {
   auto colIt = m_cols.begin();
   auto hash = colIt->first.type().hash(preds.at(colIt->first)[0].val, m_hashSize);
   ++colIt;
   for(;colIt != m_cols.end(); ++colIt) 
      hash ^= colIt->first.type().hash(preds.at(colIt->first)[0].val, m_hashSize);
   //cout << "Iterator with hash = " << hash << endl;
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
         if(status == 0xee) continue;
         if(status != 0xaa) return rowiterator(this);
         colIt = m_cols.begin();
         for(; colIt != m_cols.end(); ++colIt) {
            if(!colIt->first.type().satisfies(preds.at(colIt->first), *page, offset)) break;
         }
         if(colIt == m_cols.end()) {
            auto row = page->at<rowcount_t>(offset);
            if(m_unique) return rowiterator(this, make_pair(row, pageOffset + page->number() * pageSize));
            return rowiterator(this, HashIterator(*page, offset, preds, m_cols, m_recordSize), make_pair(row, pageOffset));
         }
      }
      page.reset(new Page(page->db(), page->name(), page->number() + 1));
      pageOffset = 0;
   }
}
