Metadata const& Database::metadata() const { return m_meta; }
Cache<File>& Database::filesCache() { return m_filesCache; }
Cache<Page>& Database::pagesCache() { return m_pagesCache; }
