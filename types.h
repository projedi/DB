#pragma once

#include <iostream>
#include <sstream>
#include <cstdint>
#include <cstring>

#include "page.h"

// A tad less silly than previously

typedef uint8_t typesize_t;
typedef uint16_t typeid_t;

struct Predicate {
   enum OPER { EQ, NEQ, LT, GT, LEQ, GEQ };
   inline Predicate(OPER, uint8_t const*);
   OPER op;
   uint8_t const* val;
};

struct SqlType {
   virtual ~SqlType() { }
   virtual typesize_t size() const = 0;
   virtual typeid_t id() const = 0;
   virtual void fromString(std::istream&, Page&, pagesize_t& offset) const = 0;
   virtual void toString(std::ostream&, Page const&, pagesize_t& offset) const = 0;
   virtual bool satisfies(std::vector<Predicate> const&, Page const&, pagesize_t) const = 0;
};

struct IntType: SqlType {
   inline typesize_t size() const;
   inline typeid_t id() const;
   inline void fromString(std::istream&, Page&, pagesize_t& offset) const;
   inline void toString(std::ostream&, Page const&, pagesize_t& offset) const;
   inline bool satisfies(std::vector<Predicate> const&, Page const&, pagesize_t) const;
};

struct DoubleType: SqlType {
   inline typesize_t size() const;
   inline typeid_t id() const;
   inline void fromString(std::istream&, Page&, pagesize_t& offset) const;
   inline void toString(std::ostream&, Page const&, pagesize_t& offset) const;
   inline bool satisfies(std::vector<Predicate> const&, Page const&, pagesize_t) const;
};

struct VarcharType: SqlType {
   inline VarcharType(typesize_t size);
   inline typesize_t size() const;
   inline typeid_t id() const;
   // TODO: These two appear to be too damn slow.
   void fromString(std::istream&, Page&, pagesize_t& offset) const;
   void toString(std::ostream&, Page const&, pagesize_t& offset) const;
   inline bool satisfies(std::vector<Predicate> const&, Page const&, pagesize_t) const;
private:
   typesize_t m_size;
};

inline SqlType* identifyType(typeid_t);

#include "types_impl.h"
