#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <cstdint>

using std::vector;
using std::map;
using std::ostream;
using std::string;
using std::pair;

struct SQLType {
   size_t size() const;
   string toString(void* data) const;
   void fromString(string const& val, void* data) const;
};

struct Column {
   Column(string const& name, size_t offset, SQLType const& type): m_name(name), m_offset(offset), m_type(type) { }
   string const& name() const { return m_name; }
   size_t offset() const { return m_offset; }
   SQLType const& type() const { return m_type; }
private:
   string m_name;
   size_t m_offset;
   SQLType m_type;
};

// Has iterator over columns.
struct TableMetadata {
   TableMetadata(string const& name, vector<pair<string, SQLType>> const& columns);
   string const& name() const { return m_name; }
   size_t rowCount() const { return m_rowCount; }
   size_t& rowCount() { return m_rowCount; }
   size_t pageCount() const { return m_pageCount; }
   size_t& pageCount() { return m_pageCount; }
   size_t rowSize() const { return m_rowSize; }
   // Copying vector stuff out
   vector<Column>::const_iterator begin() const { return m_columns.begin(); }
   vector<Column>::const_iterator end() const { return m_columns.end(); }
   //vector<Column>::iterator begin() { return m_columns.begin(); }
   //vector<Column>::iterator end() { return m_columns.end(); }
   Column const& operator[](string const& name) const;
private:
   string m_name;  
   size_t m_rowCount;
   size_t m_pageCount;
   size_t m_rowSize;
   vector<Column> m_columns;
   vector<string> m_indexes;
};

// TODO: Make it a proper singleton
struct Metadata {
   static void create(string const& filename);
   static void destroy();
   static Metadata& instance() { return *m_instance; }
   size_t pageSize() const { return m_pageSize; }
   vector<TableMetadata>::iterator begin() { return m_tables.begin(); }
   vector<TableMetadata>::const_iterator begin() const { return m_tables.begin(); }
   vector<TableMetadata>::iterator end() { return m_tables.end(); }
   vector<TableMetadata>::const_iterator end() const { return m_tables.end(); }
   vector<TableMetadata>::iterator find(string const& name);
   vector<TableMetadata>::const_iterator find(string const& name) const;
   void insert(TableMetadata const&);
   void flush();
private:
   Metadata(string const& filename);
   static Metadata* m_instance;
   vector<TableMetadata> m_tables;
   string m_filename;
   size_t m_pageSize;
};

// TODO: Make it a proper singleton
struct PageManager {
   static PageManager& instance() { return *m_instance; }
   uint8_t* getPage(string const& name, size_t number);
   uint8_t* createPage(string const& name, size_t number);
   void flushPage(string const& name, size_t number);
private:
   PageManager(string const& directory, size_t pageCount);
   static PageManager* m_instance;
};

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
void printTableRow(ostream& out, TableMetadata const& table, size_t row, vector<string> const& columns = vector<string>()) {
   Metadata metadata = Metadata::instance();
   size_t rowsPerPage = metadata.pageSize() / table.rowSize();
   size_t pageNum = row / rowsPerPage;
   uint8_t* data = (PageManager::instance()).getPage(table.name(), pageNum);
   size_t pageOffset = (row - pageNum * rowsPerPage) * table.rowSize();
   data += pageOffset;
   if(columns.empty()) {
      auto col = table.begin();
      out << col->type().toString(data + col->offset());
      for(++col; col != table.end(); ++col) 
         out << ", " << col->type().toString(data + col->offset());
   } else {
      auto colName = columns.begin();
      Column const& col = table[*colName];
      out << col.type().toString(data + col.offset());
      for(++colName; colName != columns.end(); ++colName) {
         Column const& col = table[*colName];
         out << ", " << col.type().toString(data + col.offset());
      }
   }
}

int selectAll(string const& name, ostream& out) {
   Metadata metadata = Metadata::instance();
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
   size_t rowsPerPage = metadata.pageSize() / table->rowSize();
   size_t pageNum = rowNum / rowsPerPage;
   uint8_t* data;
   if(pageNum >= table->pageCount()) {
      data = pageManager.createPage(name, pageNum); 
      table->pageCount() += 1;
   } else
      data = pageManager.getPage(name, pageNum);
   size_t pageOffset = (rowNum - pageNum * rowsPerPage) * table->rowSize();
   data += pageOffset;
   for(auto col: *table) {
      auto val = values.find(col.name());
      string valStr = "";
      if(val != values.end())
         valStr = val->second;
      col.type().fromString(valStr, data + col.offset());
   }
   pageManager.flushPage(name, pageNum);
   metadata.flush();
   return 0;
}
