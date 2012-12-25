#include "cmdlist.h"
#include "metadata.h"

using std::ostream;
using std::endl;

typedef vector<Column>::const_iterator coliter;

void printHeader(ostream& ost, Table const& table, vector<coliter> const& cols) {
   auto col = cols.begin();
   ost << (*col)->name();
   for(++col; col != cols.end(); ++col)
      ost << ", " << (*col)->name();
}

void printRow(ostream& ost, Table const& table, size_t row, vector<coliter> const& cols) {
   size_t rowsPerPage = Metadata::instance().pageSize() / table.rowSize();
   size_t pageNum = row / rowsPerPage;
   Page page("table-" + table.name(), pageNum);
   size_t pageOffset = (row - pageNum * rowsPerPage) * table.rowSize();
   auto col = cols.begin();
   ost << (*col)->type()->toString(page, pageOffset + (*col)->offset());
   for(++col; col != cols.end(); ++col)
      ost << ", " << (*col)->type()->toString(page, pageOffset + (*col)->offset());
}

int selectAll(ostream& ost, string const& name, vector<string> const& columns) {
   auto table = Metadata::instance().find(name);
   if(table == Metadata::instance().end())
      return 1;
   vector<coliter> cols;
   if(columns.empty()) {
      for(auto col = table->begin(); col != table->end(); ++col)
         cols.push_back(col);
   } else {
      for(auto col: columns) {
         auto it = table->findCol(col);
         // TODO: Report which column not found
         if(it == table->end())
            return 1;
         cols.push_back(it);
      }
   }
   printHeader(ost, *table, cols);
   for(size_t row = 0; row != table->rowCount(); ++row) {
      ost << endl;
      printRow(ost, *table, row, cols);
   }
   return 0;
}
