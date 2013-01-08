#include "cmdlist.h"

using std::ostream;
using std::endl;
using std::vector;
using std::string;

void printHeader(ostream& ost, Table const& table, vector<Column> const& cols) {
   auto col = cols.begin();
   ost << col->name();
   for(++col; col != cols.end(); ++col)
      ost << ", " << col->name();
}

Page* getPage(Database& db, Table const& table, rowcount_t row, pagesize_t& pageOffset) {
   Page* page;
   // Let's not forget dirty byte.
   pagesize_t rowSize = 1 + table.rowSize();
   pagesize_t pageSize = db.metadata().pageSize;
   rowcount_t rowsOnFirstPage = (pageSize - table.headerSize()) / rowSize;
   rowcount_t rowsOnPage = pageSize / rowSize;
   if(row < rowsOnFirstPage) {
      page = new Page(table.page());
      pageOffset = table.headerSize() + row * rowSize;
   } else {
      // It's like numbering pages from 1.
      row -= rowsOnFirstPage;
      pagenumber_t pageNum = row / rowsOnPage + 1;
      page = new Page(db, table.page().name(), pageNum);
      pageOffset = (row - (pageNum - 1) * rowsOnPage) * rowSize;
   }
   return page;
}

void printRow(Database& db, ostream& ost, Table const& table, rowcount_t row,
      vector<Column> const& cols) {
   pagesize_t pageOffset;
   Page* page = getPage(db, table, row, pageOffset);
   uint8_t res = page->at<uint8_t>(pageOffset);
   if(res != 0xaa) { delete page; return; }
   auto col = cols.begin();
   pagesize_t offset = pageOffset + col->offset();
   col->toString(ost, *page, offset);
   for(++col; col != cols.end(); ++col) {
      offset = pageOffset + col->offset();
      ost << ", ";
      col->toString(ost, *page, offset);
   }
   delete page;
}

void selectAll(Database& db, ostream& ost, Table const& table,
      vector<string> const& columns) {
   vector<Column> cols;
   if(columns.empty()) {
      for(auto col = table.begin(); col != table.end(); ++col)
         cols.push_back(*col);
   } else {
      for(auto col = columns.begin(); col != columns.end(); ++col) {
         auto it = table.find(*col);
         if(it) cols.push_back(*it);
      }
   }
   printHeader(ost, table, cols);
   for(rowcount_t row = 0; row != table.rowCount(); ++row) {
      ost << endl;
      printRow(db, ost, table, row, cols);
   }
}
