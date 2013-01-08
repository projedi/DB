#pragma once

#include <iostream>

#include "page.h"

// A tad less silly than previously

typedef uint8_t typesize_t;
typedef uint16_t typeid_t;

struct SqlType {
   virtual ~SqlType() { }
   virtual typesize_t size() const = 0;
   virtual typeid_t id() const = 0;
   virtual void fromString(std::istream&, Page&, pagesize_t& offset) const = 0;
   virtual void toString(std::ostream&, Page const&, pagesize_t& offset) const = 0;
};

struct IntType: SqlType {
   inline typesize_t size() const;
   inline typeid_t id() const;
   inline void fromString(std::istream&, Page&, pagesize_t& offset) const;
   inline void toString(std::ostream&, Page const&, pagesize_t& offset) const;
};

struct DoubleType: SqlType {
   inline typesize_t size() const;
   inline typeid_t id() const;
   inline void fromString(std::istream&, Page&, pagesize_t& offset) const;
   inline void toString(std::ostream&, Page const&, pagesize_t& offset) const;
};

struct VarcharType: SqlType {
   inline VarcharType(typesize_t size);
   inline typesize_t size() const;
   inline typeid_t id() const;
   // TODO: This two appear to be too damn slow.
   void fromString(std::istream&, Page&, pagesize_t& offset) const;
   void toString(std::ostream&, Page const&, pagesize_t& offset) const;
private:
   typesize_t m_size;
};

inline SqlType* identifyType(typeid_t);

#include "types_impl.h"
