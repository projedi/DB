#include <string>
#include <cstdint>
#include <iostream>
#include <cstring>

using namespace std;

#include "pagemanager.h"
#include "types.h"
#include "metadata.h"
#include "cmdlist.h"

void testPages() { }

void testMetadata() {
   pair<string, SqlType*> cols1[] =
      { make_pair("col1", new IntType()), make_pair("col2", new VarcharType(15)) };
   auto cols1End = cols1 + 2;
   Table t1("table1", vector<pair<string, SqlType*>>(cols1, cols1End));
   Metadata::instance().insert(t1);
   Table t2 = *(Metadata::instance().find("table2"));
   cout << t2.name() << " " << t2[1].name() << endl;
}

void testDB() {
   map<string, string> vals;
   vals["col1"] = "123";
   vals["col2"] = "qewrty";
   int res = insertInto("table1", vals);
   cout << res;
}

int main() {
   Metadata::create("test/metadata", 20);
   PageManager::create("test", 3, Metadata::instance().pageSize());
   //testPages();
   //testMetadata();
   testDB();
   PageManager::destroy();
   Metadata::destroy();
}
