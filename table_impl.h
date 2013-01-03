Column::Column(std::string const& name, size_t offset, SqlType const* type):
   m_type(type), m_name(name), m_offset(offset) { }

std::string const& Column::name() const { return m_name; }
size_t Column::offset() const { return m_offset; }
size_t& Column::offset() { return m_offset; }
size_t Column::size() const { return m_type->size(); }
size_t Column::sizeHeader() const { return 1 + m_name.size() + 2 + 1 + 8; }

template<>
inline void Page::at<Column>(uint64_t& offset, Column const& val) {
   at<std::string>(offset, val.m_name);
   at<uint16_t>(offset, val.m_type->id());
   at<uint8_t>(offset, 0); // TODO: index type
   at<uint64_t>(offset, 0); // TODO: index extra data
}

template<>
inline Column Page::at<Column>(uint64_t& offset) const {
   Column res; 
   res.m_name = at<std::string>(offset);
   uint16_t id = at<uint16_t>(offset);
   res.m_type.reset(identifyType(id));
   // TODO: Indexes:
   uint8_t idxType = at<uint8_t>(offset);
   uint64_t idxData = at<uint64_t>(offset);
   return res;
}

void Column::fromString(std::istream& ist, Page& page, uint64_t& offset) const {
   m_type->fromString(ist, page, offset);
}

void Column::toString(std::ostream& ost, Page const& page, uint64_t& offset) const {
   m_type->toString(ost, page, offset);
}

Table::Table(Database const& db, std::string const& name,
   std::vector<std::pair<std::string, SqlType*>> const& cols):
   m_page(db, "table-" + name, 0), m_name(name), m_rowCount(0) {
   m_headerSize = sizeof(uint64_t) + 1;
   m_rowSize = 0;
   for(auto col = cols.begin(); col != cols.end(); ++col) {
      Column newCol(col->first,m_rowSize,col->second);
      m_cols.push_back(newCol);
      m_rowSize += newCol.size();
      m_headerSize += newCol.sizeHeader();
   }
   saveHeader();
}

Table::~Table() { saveHeader(); }

std::string const& Table::name() const { return m_name; }
size_t& Table::rowCount() { return m_rowCount; }
size_t Table::rowCount() const { return m_rowCount; }
size_t Table::rowSize() const { return m_rowSize; }
size_t Table::headerSize() const { return m_headerSize; }
Page const& Table::page() const { return m_page; }

boost::optional<Column> Table::find(std::string const& name) const {
   for(auto col = m_cols.begin(); col != m_cols.end(); ++col) {
      if(col->name() == name) return boost::optional<Column>(*col);
   }
   return boost::optional<Column>();
}

Table::const_iterator Table::begin() const { return m_cols.begin(); }
Table::const_iterator Table::end() const { return m_cols.end(); }
Column const& Table::operator[](int idx) const { return m_cols[idx]; }

void Table::saveHeader() {
   size_t offset = 0;
   m_page.at<uint64_t>(offset, m_rowCount);
   m_page.at<uint8_t>(offset, m_cols.size());
   for(auto col = m_cols.begin(); col != m_cols.end(); ++col)
      m_page.at<Column>(offset, *col);
}


void Table::loadHeader() {
   size_t offset = 0;
   m_rowCount = m_page.at<uint64_t>(offset);
   uint8_t colSize = m_page.at<uint8_t>(offset);
   m_cols.clear();
   m_cols.reserve(colSize);
   m_rowSize = 0;
   for(uint8_t i = 0; i != colSize; ++i) {
      Column col = m_page.at<Column>(offset);
      col.offset() = m_rowSize;
      m_cols.push_back(col);
      m_rowSize += col.size();
   }
   m_headerSize = offset;
}

Table::Table(Database const& db, std::string const& name):
   m_page(db, "table-" + name, 0), m_name(name) { loadHeader(); }
