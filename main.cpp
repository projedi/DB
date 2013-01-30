#include <iostream>
#include <chrono>

#include "cmdlist.h"

using namespace std;

//TODO: Implement all stuff without indexes:
// 1. SELECT WHERE a = b, a < b, ...
// 2. UPDATE a = b where c < d, ...
// 3. DELETE where e > f
// Only then add HASH and try to bake it in

void testPages(Database& db) {
   string name = "page";
   Page** p = new Page*[8];
   for(int i = 0; i != 8; ++i) p[i] = nullptr;
   for(int i = 0; i != 1000000; ++i) {
      pagesize_t j = i % 8;
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

void testMeta(Database& db) {
   auto res = Table::findTable(db, "table1");
   if(res) { cout << "Table found" << endl; return; }
   cout << "Table not found" << endl;
   pair<string,SqlType*> cols[] =
      { make_pair("col1", new IntType()), make_pair("col2", new VarcharType(10)) };
   Table t1(db, "table1", vector<pair<string,SqlType*>>(cols, cols + 2));
   res = Table::findTable(db, "table1");
   if(res) cout << "Table found" << endl;
}

void testDB(Database& db) {
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

void insertToDB(Database& db) {
   auto res = Table::findTable(db, "table1");
   map<string, string> vals;
   for(int j = 0; j != 100000; ++j) {
      for(int i = 0; i != 12; ++i) {
         stringstream str;
         str << i;
         //cout << "putting " << str.str() << endl;
         vals["col1"] = str.str();
         if(i % 3) vals["col2"] = "def";
         else vals["col2"] = "abc";
         insertInto(db, *res, vals);
      }
   }
}

void createDB(Database& db) {
   auto res = Table::findTable(db, "table1");
   if(res) { cout << "No create" << endl; return; }
   pair<string,SqlType*> cols[] =
      { make_pair("col1", new IntType()), make_pair("col2", new VarcharType(10)) };
   Table t1(db, "table1", vector<pair<string,SqlType*>>(cols, cols + 2));
}

void testWhere(Database& db) {
   int val1 = 10;
   int val2 = 3;
   string val3 = "abc";
   map<string,vector<Predicate>> constrs;
   auto& col1 = constrs["col1"];
   col1.push_back(Predicate(Predicate::LT, (uint8_t*)&val1));
   col1.push_back(Predicate(Predicate::GEQ, (uint8_t*)&val2));
   auto& col2 = constrs["col2"];
   col2.push_back(Predicate(Predicate::EQ, (uint8_t*)val3.c_str()));
   auto res = Table::findTable(db, "table1");
   //selectWhere(db, cout, *res, constrs);
   selectAll(db, cout, *res);
}

void testDelete(Database& db) {
   int val1 = 10;
   int val2 = 3;
   string val3 = "abc";
   map<string,vector<Predicate>> constrs;
   auto& col1 = constrs["col1"];
   col1.push_back(Predicate(Predicate::LT, (uint8_t*)&val1));
   col1.push_back(Predicate(Predicate::GEQ, (uint8_t*)&val2));
   auto& col2 = constrs["col2"];
   col2.push_back(Predicate(Predicate::EQ, (uint8_t*)val3.c_str()));
   auto res = Table::findTable(db, "table1");
   deleteWhere(db, *res, constrs);
}

void testUpdate(Database& db) {
   int val1 = 10;
   int val2 = 3;
   string val3 = "abc";
   map<string,vector<Predicate>> constrs;
   auto& col1 = constrs["col1"];
   col1.push_back(Predicate(Predicate::LT, (uint8_t*)&val1));
   col1.push_back(Predicate(Predicate::GEQ, (uint8_t*)&val2));
   auto& col2 = constrs["col2"];
   col2.push_back(Predicate(Predicate::EQ, (uint8_t*)val3.c_str()));
   map<string, string> vals;
   vals["col2"] = "ae";
   auto res = Table::findTable(db, "table1");
   updateWhere(db, *res, constrs, vals);
}

void benchmark(string tag = "") {
   using namespace std::chrono;
   static high_resolution_clock::time_point t1 = high_resolution_clock::now();
   static high_resolution_clock::time_point t2 = t1;
   if(tag.empty()) return;
   high_resolution_clock::time_point t3 = high_resolution_clock::now();
   duration<double> dt1 = duration_cast<duration<double>>(t3 - t2);
   duration<double> dt2 = duration_cast<duration<double>>(t3 - t1);
   cerr << tag << ": delta = " << dt1.count() << "; total = " << dt2.count() << endl;
   t2 = t3;
}

int main() {
   Metadata meta = { "test", 3, 5, 4096 };
   Database db(meta, true);
   benchmark();
   createDB(db);
   benchmark("Create");
   insertToDB(db);
   benchmark("Insert");
   testDelete(db);
   benchmark("Delete");
   insertToDB(db);
   benchmark("Insert");
   testUpdate(db);
   benchmark("Update");
   testWhere(db);
   benchmark("Print");
}
