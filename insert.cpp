#include "cmdlist.h"

using std::string;
using std::map;
using std::stringstream;

// TODO: Has high self time. Investigate further
void insertInto(Database& db, Table& table, map<string, string> const& values) {
   rowcount_t rowNum = table.rowCount();
   table.rowCount() += 1;
   pagesize_t pageOffset;
   Page* page = getPage(db, table, rowNum, pageOffset);
   page->at<uint8_t>(pageOffset, 0xaa);
   stringstream str;
   for(auto col = table.begin(); col != table.end(); ++col) {
      auto val = values.find(col->name());
      str << val->second << '\n';
      col->fromString(str, *page, pageOffset);
   }
   delete page;
}
