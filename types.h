#pragma once

#include <iostream>
#include <sstream>
#include <cstdint>
#include <cstring>
#include <functional>

#include "page.h"

// A tad less silly than previously

typedef uint8_t typesize_t;
typedef uint16_t typeid_t;

struct Predicate {
   enum OPER { EQ, NEQ, LT, GT, LEQ, GEQ };
   template<class T> Predicate(OPER, T*);
   OPER op;
   void* val;
};

// I really don't think that hash should be here.
struct SqlType {
   virtual ~SqlType() { }
   virtual typesize_t size() const = 0;
   virtual typeid_t id() const = 0;
   virtual void write(void*, Page&, pagesize_t& offset) const = 0;
   virtual void* read(Page const&, pagesize_t& offset) const = 0;
   virtual void clear(void*) const = 0;
   virtual void toString(std::ostream&, Page const&, pagesize_t& offset) const = 0;
   virtual bool satisfies(std::vector<Predicate> const&, Page const&, pagesize_t&) const = 0;
   virtual uint32_t hash(void*, pagesize_t) const = 0;
   virtual int compare(void*, void*) const = 0;
};

struct IntType: SqlType {
   inline typesize_t size() const;
   inline typeid_t id() const;
   inline void write(void*, Page&, pagesize_t& offset) const;
   inline void* read(Page const&, pagesize_t& offset) const;
   inline void clear(void*) const;
   inline void toString(std::ostream&, Page const&, pagesize_t& offset) const;
   inline bool satisfies(std::vector<Predicate> const&, Page const&, pagesize_t&) const;
   inline uint32_t hash(void*, pagesize_t) const;
   inline int compare(void*, void*) const;
};

struct DoubleType: SqlType {
   inline typesize_t size() const;
   inline typeid_t id() const;
   inline void write(void*, Page&, pagesize_t& offset) const;
   inline void* read(Page const&, pagesize_t& offset) const;
   inline void clear(void*) const;
   inline void toString(std::ostream&, Page const&, pagesize_t& offset) const;
   inline bool satisfies(std::vector<Predicate> const&, Page const&, pagesize_t&) const;
   inline uint32_t hash(void*, pagesize_t) const;
   inline int compare(void*, void*) const;
};

struct VarcharType: SqlType {
   inline VarcharType(typesize_t size);
   inline typesize_t size() const;
   inline typeid_t id() const;
   inline void write(void*, Page&, pagesize_t& offset) const;
   inline void* read(Page const&, pagesize_t& offset) const;
   inline void clear(void*) const;
   // TODO: This one appears to be too damn slow.
   void toString(std::ostream&, Page const&, pagesize_t& offset) const;
   inline bool satisfies(std::vector<Predicate> const&, Page const&, pagesize_t&) const;
   inline uint32_t hash(void*, pagesize_t) const;
   inline int compare(void*, void*) const;
private:
   typesize_t m_size;
};

inline SqlType* identifyType(typeid_t);

#include "types_impl.h"
