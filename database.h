#pragma once

#include <string>
#include <cstdint>

#include "file.h"

struct Metadata {
   std::string path;
   cachesize_t maxPagesCount;
   cachesize_t maxFilesCount;
   pagesize_t pageSize;
};

struct Database {
   // Throws an exception if path is not a directory.
   Database(Metadata const&, bool overwrite = false);
   inline Metadata const& metadata() const;
   inline Cache<File> const& filesCache() const;
   inline Cache<Page> const& pagesCache() const;
private:
   Database(Database const&);
   Database& operator =(Database const&);
private:
   Metadata m_meta;
   // Pointers are just to allow incomplete types
   Cache<File> m_filesCache;
   Cache<Page> m_pagesCache;
};

#include "database_impl.h"
