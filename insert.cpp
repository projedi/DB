#include "cmdlist.h"
#include "metadata.h"
#include "pagemanager.h"

using std::string;

int insertInto(string const& name, map<string, string> const& values) {
   Metadata& metadata = Metadata::instance();
   auto table = Metadata::instance().find(name);
   if(table == metadata.end())
      return 1;
   size_t rowNum = table->rowCount();
   table->rowCount() += 1;
   size_t rowsPerPage = Metadata::instance().pageSize() / table->rowSize();
   size_t pageNum = rowNum / rowsPerPage;
   Page page(name, pageNum);
   size_t pageOffset = (rowNum - pageNum * rowsPerPage) * table->rowSize();
   for(auto col: *table) {
      auto val = values.find(col.name());
      string valStr = "";
      if(val != values.end())
         valStr = val->second;
      col.type()->fromString(valStr, page, pageOffset + col.offset());
   }
   Metadata::instance().flush();
   return 0;
}
