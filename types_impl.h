typesize_t IntType::size() const { return sizeof(int32_t); }
typeid_t IntType::id() const { return 1; }
void IntType::fromString(std::istream& ist, Page& page, pagesize_t& offset) const {
   int32_t res;
   ist >> res;
   page.at<int32_t>(offset, res);
}
void IntType::toString(std::ostream& ost, Page const& page, pagesize_t& offset) const {
   ost << page.at<int32_t>(offset); 
}

typesize_t DoubleType::size() const { return sizeof(double); }
typeid_t DoubleType::id() const { return 2; }
void DoubleType::fromString(std::istream& ist, Page& page, pagesize_t& offset) const {
   double res;
   ist >> res;
   page.at<double>(offset, res);
}
void DoubleType::toString(std::ostream& ost, Page const& page, pagesize_t& offset) const {
   ost << page.at<double>(offset); 
}

VarcharType::VarcharType(typesize_t size): m_size(size) { }
typesize_t VarcharType::size() const { return m_size; }
// By specification string can't be longer than 128
typeid_t VarcharType::id() const { return 3 + 256 * m_size; }

SqlType* identifyType(typeid_t id) {
   if(id == 1) return new IntType();
   if(id == 2) return new DoubleType();
   if(!((id - 3) % 256)) return new VarcharType((id - 3)/256);
   return nullptr;
}
