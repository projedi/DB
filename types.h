#pragma once

#include <cstdint>
#include <sstream>

struct SQLType {
   virtual size_t size() const = 0;
   virtual string toString(Page page, size_t offset) const = 0;
   virtual void fromString(string const& val, Page page, size_t offset) const = 0;
};

struct IntType : SQLType {
   size_t size() const { return sizeof(uint64_t); }
   string toString(Page page, size_t offset) const {
      uint64_t* data = (uint64_t*) (page + offset);
      std::stringstream str;
      str << *data;
      return str.str();
   }
   void fromString(string const& val, Page page, size_t offset) const {
      uint64_t* data = (uint64_t*) (page + offset);
      std::stringstream str; 
      str >> *data;
   }
};

struct DoubleType : SQLType {
   size_t size() const { return sizeof(double); }
   string toString(Page page, size_t offset) const {
      double* data = (double*) (page + offset);
      std::stringstream str;
      str << *data;
      return str.str();
   }
   void fromString(string const& val, Page page, size_t offset) const {
      double* data = (double*) (page + offset);
      std::stringstream str;
      str >> *data;
   }
};

struct VarcharType : SQLType {
   VarcharType(size_t size): m_size(size) { }
   size_t size() const { return m_size; }
   string toString(Page page, size_t offset) const {
      char* data = (char*) (page + offset); 
      return string(data, m_size);
   }
   void fromString(string const& val, Page page, size_t offset) const {
      char* data = (char*) (page + offset);
      for(int i = 0; i != m_size; ++i)
         data[i] = val[i];
   }
private:
   size_t m_size;
};
