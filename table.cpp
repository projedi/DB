#include "index.h"
#include <sstream>

// Why do I need them here? Standard says in case of const integral it's not required
uint8_t const Table::ADD_FLAG;
uint8_t const Table::DEL_FLAG;

struct SimpleRowIterator {
   boost::optional<std::pair<rowcount_t,pagesize_t>> operator() (rowiterator const& iter) {
      if(*iter >= rowCount) return boost::optional<std::pair<rowcount_t,pagesize_t>>();
      return boost::optional<std::pair<rowcount_t,pagesize_t>>(std::make_pair(*iter + 1, 0));
   }
   rowcount_t rowCount;
};

rowiterator Table::rowIterator() const {
   SimpleRowIterator iter;
   iter.rowCount = m_rowCount;
   return rowiterator(nullptr, iter, std::make_pair(0,0));
}

void Table::loadIndexes() {
   for(int i = 0;; ++i) {
      std::stringstream str;
      str << m_name << "-" << i;
      auto index = Index::findIndex(m_page.db(), str.str());
      if(!index) break;
      m_indexes.push_back(index);
   }
}
