#include <string>
#include <vector>
#include <map>
#include <ostream>

using std::vector;
using std::map;
using std::ostream;
using std::string;

struct SQLType {
   size_t size();
   string toString(void* data);
   void fromString(string const& val, void* data);
};

struct Column {
   string name;
   size_t offset;
   SQLType type;
}

// Has iterator over columns.
struct TableMetadata {
   TableMetadata(string const& name, vector<pair<string, SQLType>> const& columns);
   string name;  
   vector<pair<string, SQLType>> columns;
   vector<string> indexes;
   size_t rowCount;
   size_t rowSize;
};

// TODO: For memory management turn it to singleton
struct Metadata {
   Metadata(string const& filename);
   saveToDisk();

   vector<TableMetadata> tables;
};

// TODO: Make it a singleton as well
struct PageManager {
   PageManager(string const& directory, size_t pageCount);
   void* getPage(string const& name, size_t number);
   void* createPage(string const& name, size_t number);

};

// Pointer is to emulate optional value
TableMetadata* findTable(const string& name) {

}

// Trusts that columns exist in a table
void printTableHeader(ostream& out, TableMetadata const& table, vector<string> const& columns = vector<string>()) {
   if(columns.empty()) {
      auto col = table.begin();
      out << col->name();
      for(++col; col != table.end(); ++col)
         out << ", " << col->name();
   } else {
      auto colName = columns.begin();
      out << table[*colName].name();
      for(++colName; colName != columns.end(); ++colName)
         out << ", " << table[*colName].name();
   }
}

// Trusts that columns exist in a table
void printTableRow(ostream& out, TableMetadata const& table, size_t row, vector<string> const& columns = vector<string()) {
   size_t pageNum = row / table->rowsPerPage();
   void* data = (PageManager::instance()).getPage(name, pageNum);
   size_t pageOffset = (row - pageNum * table->rowsPerPage()) * table->rowSize();
   data += pageOffset;
   if(columns.empty()) {
      auto col = table.begin();
      out << col->type().toString(data + col->offset());
      for(++col; col != table.end(); ++col) 
         out << ", " << col->type().toString(data + col->offset());
   } else {
      auto colName = columns.begin();
      Column& col = table[*colName];
      out << col.type().toString(data + col.offset());
      for(++colName; colName != columns.end(); ++colName) {
         Column& col = table[*colName];
         out << ", " << col.type().toString(data + col.offset());
      }
   }
}

int selectAll(string const& name, ostream& out) {
   Metadata metadata = Metadata::instance();
   PageManager pageManager = PageManager::instance();
   auto table = metadata.find(name);
   if(table == metadata.end())
      return 1;
   printTableHeader(out, *table);
   for(size_t row = 0; row != table->rowCount(); ++row)
      printTableRow(out, *table, row);
   return 0;
}

int createTable(string const& name, vector<pair<string,SQLType>> const& columns) {
   Metadata metadata = Metadata::instance();
   TableMetadata table(name, columns);
   metadata.insert(table);
   metadata.flush();
   return 0;
}

int insertInto(string const& name, map<string, string> const& values) {
   PageManager pageManager = PageManager::instance();
   Metadata metadata = Metadata::instance();
   auto table = metadata.find(name);
   if(table == metadata.end())
      return 1;
   size_t rowNum = table->rowCount();
   table->rowCount() += 1;
   size_t pageNum = rowNum / table->rowsPerPage();
   void* data;
   if(pageNum >= table->pageCount()) {
      data = pageManager.createPage(name, pageNum); 
      table->pageCount() += 1;
   } else
      data = pageManager.getPage(name, pageNum);
   size_t pageOffset = (rowNum - pageNum * table->rowsPerPage()) * table->rowSize();
   data += pageOffset;
   for(auto col: table) {
      auto val = values.find(col->name());
      string valStr = "";
      if(val != map::end())
         valStr = val->second;
      col->type().fromString(valStr, data + col->offset());
   }
   pageManager.flushPage(name, pageNum);
   metadata.flush();
   return 0;
}
