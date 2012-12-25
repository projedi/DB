#pragma once

#include "types.h"
#include "pagemanager.h"

#include <iostream>
using namespace std;

#include <fstream>
#include <vector>
#include <algorithm>

using std::vector;

// Yes, I really need ref counter here. This IS Sparta, apparently.
struct Column {
   Column(string const& name, size_t offset, SqlType* type):
      m_name(name), m_offset(offset), m_type(type), m_refCounter(new int(1)) { }
   Column(Column const& copy):
      m_name(copy.m_name), m_offset(copy.m_offset), m_type(copy.m_type),
      m_refCounter(copy.m_refCounter) { ++(*m_refCounter); }
   ~Column() {
      --(*m_refCounter);
      if(!*m_refCounter) {
         delete m_type;
         delete m_refCounter;
      }
   }
   void swap(Column& other) {
      std::swap(m_name, other.m_name);
      std::swap(m_offset, other.m_offset);
      std::swap(m_type, other.m_type);
      std::swap(m_refCounter, other.m_refCounter);
   }
   Column& operator =(Column const& copy) {
      Column other = copy;
      swap(other);
      return *this;
   }
   string const& name() const { return m_name; }
   size_t offset() const { return m_offset; }
   SqlType const* type() const { return m_type; }
private:
   string m_name;
   size_t m_offset;
   SqlType* m_type;
   int* m_refCounter;
};

// Has iterator over columns.
struct Table {
   Table(): m_rowCount(0), m_rowSize(0) { }
   Table(string const& name, std::vector<std::pair<string, SqlType*>> const& columns);
   string const& name() const { return m_name; }
   size_t rowCount() const { return m_rowCount; }
   size_t& rowCount() { return m_rowCount; }
   size_t rowSize() const { return m_rowSize; }
   vector<Column>::const_iterator begin() const { return m_columns.begin(); }
   vector<Column>::const_iterator end() const { return m_columns.end(); }
   Column const& operator[](int idx) const { return m_columns[idx]; }
   vector<Column>::const_iterator findCol(string const& name) const;
   friend std::istream& operator >>(std::istream&, Table&);
   friend std::ostream& operator <<(std::ostream&, Table const&);
private:
   string m_name;  
   size_t m_rowCount;
   size_t m_rowSize;
   vector<Column> m_columns;
   //vector<string> m_indexes;
};

struct Metadata {
   // Only uses 2nd param when file creation required
   static void create(string const& filename, size_t pageSize = 0)
      { m_instance = new Metadata(filename, pageSize); }
   static void destroy() { delete m_instance; }
   static Metadata& instance() { return *m_instance; }
   size_t pageSize() const { return m_pageSize; }
   vector<Table>::iterator begin() { return m_tables.begin(); }
   vector<Table>::iterator end() { return m_tables.end(); }
   vector<Table>::iterator find(string const& name);
   void insert(Table const& table) { m_tables.push_back(table); }
   void flush();
private:
   Metadata(string const& filename, size_t pageSize);
   ~Metadata() { flush(); }
   Metadata(Metadata const&);
   Metadata& operator=(Metadata const&);
private:
   static Metadata* m_instance;
   vector<Table> m_tables;
   size_t m_pageSize;
   string m_filename;
};
