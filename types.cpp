#include "types.h"

void VarcharType::fromString(std::istream& ist, Page& page, pagesize_t& offset) const {
   std::string str;
   ist >> str;
   char const* strVal = str.data();
   typesize_t size = (typesize_t)str.size();
   for(typesize_t i = 0; i != m_size; ++i, ++strVal) {
      if(i < size) page.at<char>(offset, *strVal);
      else page.at<char>(offset, 0);
   }
}

void VarcharType::toString(std::ostream& ost, Page const& page, pagesize_t& offset) const {
   ost << '"';
   for(typesize_t i = 0; i != m_size; ++i) {
      char c = page.at<char>(offset);
      if(c) ost << c;
   }
   ost << '"';
}
