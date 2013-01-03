#include "types.h"

struct VarcharValue {
   VarcharValue(uint8_t size, std::istream& ist): m_ist(ist), m_size(size) { }
   std::istream& m_ist;
   uint8_t m_size;
};

template<>
inline void Page::at<VarcharValue>(uint64_t& offset, VarcharValue const& val) {
   if(!buffer()) loadPage();
   char* data = (char*)(buffer() + offset);
   std::string str;
   val.m_ist >> str;
   uint8_t size = str.size();
   for(uint8_t i = 0; i != val.m_size; ++i, ++data) {
      if(i < size) *data = str[i];
      else *data = 0;
   }
   offset += val.m_size;
}

void VarcharType::fromString(std::istream& ist, Page& page, uint64_t& offset) const {
   VarcharValue res(m_size, ist);
   page.at<VarcharValue>(offset, res);
}

void VarcharType::toString(std::ostream& ost, Page const& page, uint64_t& offset) const {
   ost << '"';
   for(uint8_t i = 0; i != m_size; ++i) {
      ost << page.at<char>(offset);
   }
   ost << '"';
}
