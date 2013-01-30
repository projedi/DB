#include "types.h"

void VarcharType::toString(std::ostream& ost, Page const& page, pagesize_t& offset) const {
   ost << '"';
   for(typesize_t i = 0; i != m_size; ++i) {
      char c = page.at<char>(offset);
      if(c) ost << c;
   }
   ost << '"';
}
