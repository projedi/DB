Metadata const& Database::metadata() const { return m_meta; }
Cache<File>& Database::filesCache() const { return *m_filesCache; }
Cache<Page>& Database::pagesCache() const { return *m_pagesCache; }
