#include <string>
#include <cstdint>
#include <iostream>
#include <cstring>

using namespace std;

#include "pagemanager.h"
#include "types.h"
#include "metadata.h"
#include "cmdlist.h"

void testDB() {
   int res;
   pair<string, SqlType*> cols[] =
      { make_pair("col1", new IntType()), make_pair("col2", new VarcharType(10)) };
   createTable("table1", vector<pair<string,SqlType*>>(cols,cols+2));
   map<string, string> vals;
   vals["col1"] = "123";
   vals["col2"] = "qewrty";
   res = insertInto("table1", vals);
   cout << res << endl;
   res = selectAll(cout, "table1");
   cout << endl << res << endl;
}

int main() {
   Metadata::create("test/metadata", 20);
   PageManager::create("test", 3, Metadata::instance().pageSize());
   testDB();
   PageManager::destroy();
   Metadata::destroy();
}
