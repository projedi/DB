Column::Column(std::string const& name, pagesize_t offset, SqlType const* type):
   m_type(type), m_name(name), m_offset(offset) { }

std::string const& Column::name() const { return m_name; }
pagesize_t Column::offset() const { return m_offset; }
pagesize_t& Column::offset() { return m_offset; }
pagesize_t Column::size() const { return m_type->size(); }
SqlType const& Column::type() const { return *m_type; }
pagesize_t Column::sizeHeader() const { return 1 + m_name.size() + sizeof(typeid_t); }
bool Column::operator <(Column const& other) const { return m_name < other.m_name; }

template<>
inline void Page::at<Column>(pagesize_t& offset, Column const& val) {
   at<std::string>(offset, val.m_name);
   at<typeid_t>(offset, val.m_type->id());
}

template<>
inline Column Page::at<Column>(pagesize_t& offset) const {
   std::string name = at<std::string>(offset);
   typeid_t id = at<typeid_t>(offset);
   SqlType* type = identifyType(id);
   Column res(name, 0, type); 
   return res;
}

Table::Table(Database const* db, std::string const& name,
      std::vector<InputColumn> const& cols):
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

boost::optional<Column> Table::findColumn(std::string const& name) const {
   for(auto col = m_cols.begin(); col != m_cols.end(); ++col) {
      if(col->name() == name) return boost::optional<Column>(*col);
   }
   return boost::optional<Column>();
}

std::vector<Column> const& Table::cols() const { return m_cols; }

void Table::addIndex(std::shared_ptr<Index const> index) { m_indexes.push_back(index); }

std::vector<std::shared_ptr<Index const>> const& Table::indexes() const { return m_indexes; }

boost::optional<Table> Table::findTable(Database const* db, std::string const& name) {
   std::fstream f(db->metadata().path + "/" + "table-" + name, std::fstream::in);
   if(f.is_open()) return boost::optional<Table>(Table(db, name));
   else return boost::optional<Table>();
}

Table::Table(Database const* db, std::string const& name):
   m_page(db, "table-" + name, 0), m_name(name) { loadHeader(); }

void Table::saveHeader() const {
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
   loadIndexes();
}
