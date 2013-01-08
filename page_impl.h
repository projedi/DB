bool Page::operator ==(Page const& o) const {
   return m_name == o.m_name && m_number == o.m_number;
}

bool Page::operator <(Page const& o) const {
   int res = m_name.compare(o.m_name);
   return (res < 0) || (!res && m_number < o.m_number);
}

std::string const& Page::name() const { return m_name; }
pagenumber_t Page::number() const { return m_number; }

template<class T>
void Page::at(pagesize_t& offset, T const& val) {
   uint8_t* data = buffer();
   if(!data) {
      loadPage();
      data = buffer();
   }
   *(T*)(data + offset) = val;
   offset += sizeof(T);
   setDirty();
}

template<class T>
T Page::at(pagesize_t& offset) const {
   uint8_t* data = buffer();
   if(!data) {
      loadPage();
      data = buffer();
   }
   T& res = *(T*)(data + offset);
   offset += sizeof(T);
   return res;
}

template<>
inline void Page::at<std::string>(pagesize_t& offset, std::string const& val) {
   uint8_t* data = buffer();
   if(!data) {
      loadPage(); 
      data = buffer();
   }
   data += offset;
   uint8_t size = (uint8_t)val.size();
   *data = size;
   ++data;
   const char* valData = val.data();
   for(uint8_t i = 0; i != size; ++i, ++data, ++valData)
      *data = *(uint8_t*)valData;
   offset += 1 + size;
   setDirty();
}

template<>
inline std::string Page::at<std::string>(pagesize_t& offset) const {
   uint8_t* data = buffer();
   if(!data) {
      loadPage();
      data = buffer();
   }
   data += offset;
   uint8_t size = *data;
   std::string res((char*)(data + 1), size);
   offset += 1 + size;
   return res;
}
