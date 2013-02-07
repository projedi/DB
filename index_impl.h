rowiterator::rowiterator(std::shared_ptr<Index const> owner, nextf next,
      std::pair<rowcount_t,pagesize_t> init):
   m_row(init.first), m_offset(init.second), m_owner(owner), m_next(next), m_active(true) { }

rowiterator& rowiterator::operator ++() {
   if(!m_active) return *this;
   auto val = m_next(*this);    
   if(val) {
      m_row = val->first;
      m_offset = val->second;
   } else m_active = false;
   return *this;
}

rowiterator::operator bool() const { return m_active; }
rowcount_t rowiterator::operator *() const { return m_row; }
rowcount_t rowiterator::row() const { return m_row; }
pagesize_t rowiterator::offset() const { return m_offset; }
std::shared_ptr<Index const> rowiterator::owner() const { return m_owner; }
bool rowiterator::active() const { return m_active; }

Index::~Index() { }

bool Index::unique() const { return m_unique; }
Index::Type Index::type() const { return m_type; }
std::vector<std::pair<Column, Index::Direction>> const& Index::cols() const {
   return m_cols;
}

std::shared_ptr<Index> Index::findIndex(Database const* db, std::string const& name) {
   std::fstream f(db->metadata().path + "/" + "index-" + name, std::fstream::in);
   if(f.is_open()) return openIndex(db, "index-" + name);
   else return std::shared_ptr<Index>();
}

std::string Index::getAvailableName(Database const* db, Table const& table) {
   std::string filenamePrefix = db->metadata().path + "/index-" + table.name() + "-";
   int i = 0;
   for(;; ++i) {
      std::stringstream str;
      str << filenamePrefix << i;
      std::fstream f(str.str(), std::fstream::in);
      if(!f.is_open()) break;
   }
   std::stringstream str;
   str << "index-" << table.name() << "-" << i;
   return str.str();
}
