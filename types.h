#pragma once

#include <cstdint>
#include <iostream>

#include "page.h"

// A tad less silly than previously

struct SqlType {
   virtual ~SqlType() { }
   virtual uint8_t size() const = 0;
   virtual uint16_t id() const = 0;
   virtual void fromString(std::istream&, Page&, uint64_t& offset) const = 0;
   virtual void toString(std::ostream&, Page const&, uint64_t& offset) const = 0;
};

struct IntType: SqlType {
   inline uint8_t size() const;
   inline uint16_t id() const;
   inline void fromString(std::istream&, Page&, uint64_t& offset) const;
   inline void toString(std::ostream&, Page const&, uint64_t& offset) const;
};

struct DoubleType: SqlType {
   inline uint8_t size() const;
   inline uint16_t id() const;
   inline void fromString(std::istream&, Page&, uint64_t& offset) const;
   inline void toString(std::ostream&, Page const&, uint64_t& offset) const;
};

struct VarcharType: SqlType {
   inline VarcharType(uint8_t size);
   inline uint8_t size() const;
   inline uint16_t id() const;
   void fromString(std::istream&, Page&, uint64_t& offset) const;
   void toString(std::ostream&, Page const&, uint64_t& offset) const;
private:
   uint8_t m_size;
};

inline SqlType* identifyType(uint16_t);

#include "types_impl.h"
