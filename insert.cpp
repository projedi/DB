#include "cmdlist.h"

using std::string;
using std::map;
using std::stringstream;

void insertInto(Database const& db, Table& table,
      map<string, string> const& values) {
   size_t rowNum = table.rowCount();
   table.rowCount() += 1;
   size_t pageOffset;
   Page* page = getPage(db, table, rowNum, pageOffset);
   page->at<uint8_t>(pageOffset, 0xaa);
   stringstream str;
   for(auto col = table.begin(); col != table.end(); ++col) {
      auto val = values.find(col->name());
      str << val->second << '\n';
      col->fromString(str, *page, pageOffset);
   }
   delete page;
   table.saveHeader();
}
