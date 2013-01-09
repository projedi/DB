Column::Column(std::string const& name, pagesize_t offset, SqlType const* type):
   m_type(type), m_name(name), m_offset(offset) { }

std::string const& Column::name() const { return m_name; }
pagesize_t Column::offset() const { return m_offset; }
pagesize_t& Column::offset() { return m_offset; }
pagesize_t Column::size() const { return m_type->size(); }
pagesize_t Column::sizeHeader() const { return 1 + m_name.size() + 2 + 1 + 8; }

template<>
inline void Page::at<Column>(pagesize_t& offset, Column const& val) {
   at<std::string>(offset, val.m_name);
   at<typeid_t>(offset, val.m_type->id());
   at<uint8_t>(offset, 0); // TODO: index type
   at<uint64_t>(offset, 0); // TODO: index extra data
}

template<>
inline Column Page::at<Column>(pagesize_t& offset) const {
   std::string name = at<std::string>(offset);
   typeid_t id = at<typeid_t>(offset);
   SqlType* type = identifyType(id);
   Column res(name, 0, type); 
   // TODO: Indexes:
   //uint8_t idxType = at<uint8_t>(offset);
   //uint64_t idxData = at<uint64_t>(offset);
   offset += 9;
   return res;
}

void Column::fromString(std::istream& ist, Page& page, pagesize_t& offset) const {
   m_type->fromString(ist, page, offset);
}

void Column::toString(std::ostream& ost, Page const& page, pagesize_t& offset) const {
   m_type->toString(ost, page, offset);
}

Table::Table(Database& db, std::string const& name, std::vector<InputColumn> const& cols):
   m_page(db, "table-" + name, 0), m_name(name), m_rowCount(0) {
   m_headerSize = sizeof(rowcount_t) + 1;
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
rowcount_t& Table::rowCount() { return m_rowCount; }
rowcount_t Table::rowCount() const { return m_rowCount; }
pagesize_t Table::rowSize() const { return m_rowSize; }
pagesize_t Table::headerSize() const { return m_headerSize; }
Page const& Table::page() const { return m_page; }

boost::optional<Column> Table::find(std::string const& name) const {
   for(auto col = m_cols.begin(); col != m_cols.end(); ++col) {
      if(col->name() == name) return boost::optional<Column>(*col);
   }
   return boost::optional<Column>();
}

Table::const_iterator Table::begin() const { return m_cols.begin(); }
Table::const_iterator Table::end() const { return m_cols.end(); }
Column const& Table::operator[](colcount_t idx) const { return m_cols[idx]; }

void Table::saveHeader() {
   pagesize_t offset = 0;
   m_page.at<rowcount_t>(offset, m_rowCount);
   m_page.at<colcount_t>(offset, (colcount_t)m_cols.size());
   for(auto col = m_cols.begin(); col != m_cols.end(); ++col)
      m_page.at<Column>(offset, *col);
}


void Table::loadHeader() {
   pagesize_t offset = 0;
   m_rowCount = m_page.at<rowcount_t>(offset);
   colcount_t colSize = m_page.at<colcount_t>(offset);
   m_cols.clear();
   m_cols.reserve(colSize);
   m_rowSize = 0;
   for(colcount_t i = 0; i != colSize; ++i) {
      Column col = m_page.at<Column>(offset);
      col.offset() = m_rowSize;
      m_cols.push_back(col);
      m_rowSize += col.size();
   }
   m_headerSize = offset;
}

boost::optional<Table> Table::findTable(Database& db, std::string const& name) {
   std::fstream f(db.metadata().path + "/" + "table-" + name, std::fstream::in);
   if(f.is_open()) return boost::optional<Table>(Table(db, name));
   else return boost::optional<Table>();
}

Table::Table(Database& db, std::string const& name):
   m_page(db, "table-" + name, 0), m_name(name) { loadHeader(); }
