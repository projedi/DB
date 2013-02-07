Hash::Hash(Database const* db, std::string const& name, bool isUnique,
      std::vector<std::pair<Column, Direction>> const& cols):
   Index(db, name, isUnique, cols), m_type(Index::Hash) {
   m_recordSize = sizeof(uint8_t) + sizeof(rowcount_t);
   for(auto it = m_cols.begin(); it != m_cols.end(); ++it)
      m_recordSize += it->first.
   m_record
}
Hash::Hash(Database& db, std::string const& name, bucketsize_t bucketSize,
      pagesize_t hashSize, bool isUnique,
      std::vector<std::shared_ptr<SqlType>> const& key):
   m_page(db, name, 0), m_bucketSize(bucketSize), m_hashSize(hashSize),
   m_keyType(key), m_unique(isUnique) {
   m_headerSize = sizeof(uint8_t) + sizeof(bucketsize_t) + sizeof(pagesize_t) +
      sizeof(uint8_t) + m_keyType.size() * sizeof(typeid_t);
   m_recordSize = sizeof(uint8_t) + sizeof(rowcount_t);
   for(auto it = m_keyType.begin(); it != m_keyType.end(); ++it) {
      m_recordSize += (*it)->size();
   }
} 

// TODO: header size consideration
bool Hash::insertValue(std::vector<void*> const& key, rowcount_t row) {
   auto keyIt = key.begin();
   auto keyTypeIt = m_keyType.begin();
   auto hash = (*keyTypeIt)->hash(*keyIt, m_hashSize);
   for(;keyIt != key.end(); ++keyIt, ++keyTypeIt)
      hash ^= (*keyTypeIt)->hash(*keyIt, m_hashSize);
   auto pageSize = m_page.pageSize();
   auto recordsPerPage = pageSize / m_recordSize;
   auto recordIdx = hash * m_bucketSize;
   auto pageNum = recordIdx / recordsPerPage;
   auto pageOffset = (recordIdx - pageNum * recordsPerPage) * m_recordSize;
   Page page(db, m_page.name(), pageNum);
   while(true) {
      for(; pageOffset + m_recordSize <= pageSize;) {
         auto offset = pageOffset;
         auto status = page.at<uint8_t>(offset);
         if(status != 0xaa) {
            offset = pageOffset;
            page.at<uint8_t>(offset, 0xaa);
            keyIt = key.begin();
            keyTypeIt = m_keyType.begin();
            for(; keyIt != key.end(); ++keyIt, ++keyTypeIt)
               (*keyTypeIt)->write(*keyIt, page, offset);
            page.at<rowcount_t>(offset, row);
            return true;
         }
         if(m_unique) {
            keyIt = key.begin();
            keyTypeIt = m_keyType.begin();
            vector<Predicate> predicate(0, Predicate(EQ, nullptr));
            for(; keyIt != key.end(); ++keyIt, ++keyTypeIt) {
               predicate[0].val = *keyIt;
               if(!(*keyTypeIt)->satisfies(predicate, page, offset)) break;
            }
            if(keyIt == key.end()) return false;
         }
         pageOffset += m_recordSize;
      }
      ++pageNum;
      pageOffset = 0;
      page = Page(db, m_page.name(), pageNum);
   }
}

bool Hash::removeAt(pagesize_t offset) {
   pagecount_t pageNum = offset / m_page.pageSize(); 
   offset %= m_page.pageSize();
   Page page(m_page.db(), m_page.name(), pageNum);
   page.at<uint8_t>(offset, 0xee);
}

void Hash::find(std::vector<void*> const& key, std::vector<rowcount_t>& rows,
      std::vector<pagesize_t>& offsets) {
   auto keyIt = key.begin();
   auto keyTypeIt = m_keyType.begin();
   auto hash = (*keyTypeIt)->hash(*keyIt, m_hashSize);
   for(;keyIt != key.end(); ++keyIt, ++keyTypeIt)
      hash ^= (*keyTypeIt)->hash(*keyIt, m_hashSize);
   auto pageSize = m_page.pageSize();
   auto recordsPerPage = pageSize / m_recordSize;
   auto recordIdx = hash * m_bucketSize;
   auto pageNum = recordIdx / recordsPerPage;
   auto pageOffset = (recordIdx - pageNum * recordsPerPage) * m_recordSize;
   Page page(db, m_page.name(), pageNum);
   while(true) {
      for(; pageOffset + m_recordSize <= pageSize;) {
         auto offset = pageOffset;
         auto status = page.at<uint8_t>(offset);
         if(status == 0xaa) {
            keyIt = key.begin();
            keyTypeIt = m_keyType.begin();
            vector<Predicate> predicate(0, Predicate(EQ, nullptr));
            for(; keyIt != key.end(); ++keyIt, ++keyTypeIt) {
               predicate[0].val = *keyIt;
               if(!(*keyTypeIt)->satisfies(predicate, page, offset)) break;
            }
            if(keyIt == key.end()) {
               rows.push_back(page.at<rowcount_t>(offset));
               offsets.push_back(pageOffset);
               if(m_unique) return;
            }
         } else if(status == 0xee) return;
         pageOffset += m_recordSize;
      }
      ++pageNum;
      pageOffset = 0;
      page = Page(db, m_page.name(), pageNum);
   }
}

void Hash::getRows(std::vector<std::vector<Predicate>> const& predicates,
      std::vector<rowcount_t>& rows) {
   std::vector<void*> values;
   auto it = predicates.begin();
   for(; it != predicates.end(); ++it) {
      if(it->size() != 1) return;
      if((*it)[0].op != Predicate::EQ) return;
      values.push_back((*it)[0].val);
   }
   std::vector<pagesize_t> offsets;
   find(values, rows, offsets);
}

static boost::optional<Hash> load(Database& db, std::string const& name) {
   std::fstream f(db.metadata().path + "/" + name, std::fstream::in);
   if(f.is_open()) return boost::optional<Hash>(Hash(db, name));
   else return boost::optional<Hash>();
}

Hash::Hash(Database& db, std::string const& name): m_page(db, name, 0) {
   pagesize_t offset = 0;
   m_unique = (bool)page.at<uint8_t>(offset); 
   m_hashSize = page.at<pagesize_t>(offset);
   m_bucketSize = page.at<bucketsize_t>(offset);
   uint8_t columnCount = page.at<uint8_t>(offset);
   for(uint8_t i = 0; i != columnCount; ++i) {
      typeid_t type = page.at<typeid_t>(offset);
      m_keyType.push_back(std::shared_ptr<SqlType>(idenitifyType(type)));
   }
   m_headerSize = sizeof(uint8_t) + sizeof(bucketsize_t) + sizeof(pagesize_t) +
      sizeof(uint8_t) + columnCount * sizeof(typeid_t);
   m_recordSize = sizeof(uint8_t) + sizeof(rowcount_t);
   for(auto it = m_keyType.begin(); it != m_keyType.end(); ++it) {
      m_recordSize += (*it)->size();
   }
}
