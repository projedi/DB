bool Page::operator ==(Page const& o) const {
   return m_name == o.m_name && m_number == o.m_number;
}

bool Page::operator <(Page const& o) const {
   int res = m_name.compare(o.m_name);
   return (res < 0) || (!res && m_number < o.m_number);
}

std::string const& Page::name() const { return m_name; }
size_t Page::number() const { return m_number; }

template<class T>
bool Page::isValid(size_t offset) const {
   return (offset + sizeof(T)) <= m_db->metadata().pageSize;
}

template<class T>
void Page::at(uint64_t& offset, T const& val) {
   if(!buffer()) loadPage();
   *(T*)(buffer() + offset) = val;
   offset += sizeof(T);
}

template<class T>
T Page::at(uint64_t& offset) const {
   if(!buffer()) loadPage();
   T& res = *(T*)(buffer() + offset);
   offset += sizeof(T);
   return res;
}

template<>
inline void Page::at<std::string>(uint64_t& offset, std::string const& val) {
   if(!buffer()) loadPage(); 
   uint8_t* data = buffer() + offset;
   uint8_t size = (uint8_t)val.size();
   *data = size;
   ++data;
   for(uint8_t i = 0; i != size; ++i, ++data)
      *data = (uint8_t)val[i];
   offset += 1 + size;
}

template<>
inline std::string Page::at<std::string>(uint64_t& offset) const {
   if(!buffer()) loadPage();
   uint8_t* data = buffer() + offset;
   uint8_t size = *data;
   std::string res((char*)(data + 1), size);
   offset += 1 + size;
   return res;
}
