#pragma once

#include "pagemanager.h"

#include <iostream>
using namespace std;

#include <cstdint>
#include <sstream>

struct SqlType {
   virtual ~SqlType() { }
   virtual size_t size() const = 0;
   virtual string toString(Page, size_t) const = 0;
   virtual void fromString(string const&, Page, size_t) const = 0;
   virtual string id() const = 0;
};

template<class T>
struct SqlTypeGeneric: SqlType {
   size_t size() const { return sizeof(T); }
   string toString(Page page, size_t offset) const {
      T* data = (T*) (page + offset);
      std::stringstream str;
      str << *data;
      return str.str();
   }
   void fromString(string const& val, Page page, size_t offset) const {
      T* data = (T*) (page + offset);
      if(val == "") {
         *data = T();
         return;
      }
      std::stringstream str(val);
      str >> *data;
   }
};

template<>
struct SqlTypeGeneric<string>: SqlType {
   SqlTypeGeneric(size_t size): m_size(size) { }
   size_t size() const { return m_size; }
   string toString(Page page, size_t offset) const {
      char* data = (char*) (page + offset); 
      return string(data, m_size);
   }
   void fromString(string const& val, Page page, size_t offset) const {
      char* data = (char*) (page + offset);
      for(int i = 0; i != m_size; ++i) {
         if(i < val.size())
            data[i] = val[i];
         else
            data[i] = 0;
      }
   }
protected:
   size_t m_size;
};

// You probably should kill me right over here.

struct IntType: SqlTypeGeneric<int64_t> {
   string id() const { return "int"; }
};

struct DoubleType: SqlTypeGeneric<double> {
   string id() const { return "dbl"; }
};

struct VarcharType: SqlTypeGeneric<string> {
   VarcharType(size_t size): SqlTypeGeneric<string>(size) { }
   string id() const {
      std::stringstream str;
      str << "str" << m_size;
      return str.str();
   }
};

inline istream& operator >>(istream& ist, SqlType*& type) {
   string res;
   ist >> res;
   type = 0;
   if(!res.compare(0,3,"int"))
      type = new IntType(); 
   else if(!res.compare(0,3,"dbl"))
      type = new DoubleType(); 
   else if(!res.compare(0,3,"str")) {
      std::stringstream str(res.substr(3));
      size_t size = 0;
      str >> size;
      type = new VarcharType(size);
   }
   return ist;
}

inline ostream& operator <<(ostream& ost, SqlType const* type) {
   ost << type->id();
   return ost;
}
