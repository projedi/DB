// TODO: Enforce dirty bit to table list because it's broken now.
// Table list: dirty , record size(with dirty), name length, name, column count,
//             column name length, column name, column type code, ...,
//             indexes count, index of indexed column, index type code,
//             extra index data length, extra index data
// Table: dirty, data, dirty, data, ...
// Dirty shows if entry is deleted.
#include <string>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;

#include "database.h"
#include "page.h"
#include "table.h"
#include "cmdlist.h"

void testPages(Database const& db) {
   string name = "page";
   Page** p = new Page*[8];
   for(int i = 0; i != 8; ++i) p[i] = nullptr;
   for(int i = 0; i != 1000000; ++i) {
      uint64_t j = i % 8;
      if(p[j]) delete p[j];
      p[j] = new Page(db, name, i);
      char c = 'x';
      p[j]->at<char>(j, c);
   }
   for(int i = 0; i != 8; ++i) {
      if(p[i]) delete p[i];
   }
   delete [] p;
}

void testMeta(Database const& db) {
   auto res = Table::findTable(db, "table1");
   if(res) { cout << "Table found" << endl; return; }
   cout << "Table not found" << endl;
   pair<string,SqlType*> cols[] =
      { make_pair("col1", new IntType()), make_pair("col2", new VarcharType(10)) };
   Table t1(db, "table1", vector<pair<string,SqlType*>>(cols, cols + 2));
   res = Table::findTable(db, "table1");
   if(res) cout << "Table found" << endl;
}

void testDB(Database const& db) {
   auto res = Table::findTable(db, "table1");
   if(!res) { cout << "Table not found" << endl; return; }
   map<string, string> vals;
   vals["col1"] = "123";
   vals["col2"] = "qewrty";
   for(int i = 0; i != 500000; ++i) {
      insertInto(db, *res, vals);
   }
   selectAll(db, cout, *res);
}

int main() {
   // Valgrind freaks out
   //Metadata meta = { "test", 3, 5, 100 };
   Metadata meta = { "test", 3, 5, 4096 };
   Database db(meta, true);
   //testPages(db);
   testMeta(db);
   testDB(db);
}
