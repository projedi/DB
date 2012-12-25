#include "metadata.h"

using std::fstream;
using std::endl;
using std::vector;
using std::pair;

Metadata* Metadata::m_instance = 0;

Table::Table(string const& name, vector<pair<string, SqlType*>> const& cols):
   m_name(name), m_rowCount(0), m_rowSize(0) {
   for(auto col: cols) {
      m_columns.push_back(Column(col.first, m_rowSize, col.second));
      m_rowSize += col.second->size();
   }
}

vector<Column>::const_iterator Table::findCol(string const& name) const {
   for(auto col = m_columns.begin(); col != m_columns.end(); ++col) {
      if(col->name() == name)
         return col;
   }
   return end();
}

// TODO: Faster search
vector<Table>::iterator Metadata::find(string const& name) {
   for(auto table = m_tables.begin(); table != m_tables.end(); ++table)
      if(table->name() == name) return table;
   return end();
}

// TODO: More efficient serialization
std::istream& operator >>(std::istream& ist, Table& table) {
   ist >> table.m_name >> table.m_rowCount;
   int size;
   ist >> size;
   size_t offset = 0;
   for(int i = 0; i != size; ++i) {
      string name;
      SqlType* type;
      ist >> name >> type;
      Column col(name, offset, type);
      table.m_columns.push_back(col);
      offset += type->size();
   }
   table.m_rowSize = offset;
   //ist >> size;
   //for(int i = 0; i != size; ++i) {
      //string name;
      //ist >> name;
      //m_indexes.push_back(name);
   //}
   return ist;
}

// TODO: More efficient serialization
std::ostream& operator <<(std::ostream& ost, Table const& table) {
   ost << table.m_name << " " << table.m_rowCount;
   ost << endl << table.m_columns.size();
   if(table.m_columns.size()) {
      for(auto col: table.m_columns)
         ost << endl << col.name() << " " << col.type();
   }
   //ost << endl << table.m_indexes.size();
   //if(table.m_indexes.size()) {
      //for(auto index: table.m_indexes)
         //ost << endl << index;
   //}
   return ost;
}

// TODO: More efficient serialization
void Metadata::flush() {
   fstream file(m_filename, fstream::out);
   file << m_pageSize;
   if(!m_tables.size()) return;
   for(auto table: m_tables)
      file << endl << table;
}

// TODO: More efficient serialization
Metadata::Metadata(string const& filename, size_t pageSize): m_filename(filename) {
   fstream file(filename, fstream::in);
   if(!file.is_open()) {
      m_pageSize = pageSize;
      return;
   }
   file >> m_pageSize; 
   while(!file.eof()) {
      Table table;
      file >> table;
      m_tables.push_back(table);
   }
}
