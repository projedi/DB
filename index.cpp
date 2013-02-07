#include "index.h"
#include "hash.h"

Index::Index(Database const* db, std::string const& name, bool isUnique, Type type,
      std::vector<std::pair<Column, Direction>> const& cols):
   m_page(db, name, 0), m_type(type), m_unique(isUnique), m_cols(cols) {
   m_headerSize = 3;
   for(auto it = m_cols.begin(); it != m_cols.end(); ++it) {
      m_headerSize += it->first.sizeHeader() + 1;
   }
}

Index::Index(Database const* db, std::string const& name): m_page(db, name, 0) {
   loadHeader(); 
}

void Index::saveHeader() const {
   pagesize_t offset = 0;
   m_page.at<uint8_t>(offset, m_type);
   m_page.at<uint8_t>(offset, m_unique);
   m_page.at<uint8_t>(offset, m_cols.size());
   for(auto it = m_cols.begin(); it != m_cols.end(); ++it) {
      m_page.at<Column>(offset, it->first);
      m_page.at<uint8_t>(offset, it->second);
   }
}

void Index::loadHeader() {
   pagesize_t offset = 0;
   m_type = (Type)m_page.at<uint8_t>(offset);
   m_unique = (bool)m_page.at<uint8_t>(offset);
   m_cols.clear();
   auto count = m_page.at<uint8_t>(offset);
   for(auto i = 0; i != count; ++i) {
      Column col = m_page.at<Column>(offset);
      Direction dir = (Direction)m_page.at<uint8_t>(offset);
      m_cols.push_back(std::make_pair(col,dir));
   }
   m_headerSize = 3;
   for(auto it = m_cols.begin(); it != m_cols.end(); ++it) {
      m_headerSize += it->first.sizeHeader() + 1;
   }
}

std::shared_ptr<Index> Index::createIndex(Database const* db, Table const& table,
      Index::Type type, bool isUnique, std::vector<std::pair<Column, Direction>> const& cols) {
   std::shared_ptr<Index> res;
   std::string name = getAvailableName(db, table);
   if(type == Index::Hash) res.reset(new struct Hash(db, name, isUnique, cols));
   // TODO: Btree construction
   //else res.reset(new BTree(db, name, cols));
   res->saveHeader();
   return res;
}

std::shared_ptr<Index> Index::openIndex(Database const* db, std::string const& name) {
   std::shared_ptr<Index> res;
   Page page(db, name, 0);
   pagesize_t offset = 0;
   Type type = (Type)page.at<uint8_t>(offset);
   if(type == Index::Hash) res.reset(new struct Hash(db, name));
   // TODO: Btree construction
   //else res.reset(new BTree(db, name));
   return res; 
}
