#pragma once

#include <string>
#include <cstdint>

struct Metadata {
   std::string path;
   // TODO: What types are better suited here?
   uint64_t maxPagesCount;
   uint64_t maxFilesCount;
   uint64_t pageSize;
};

template<class T> struct Cache;
struct File;
struct Page;

struct Database {
   // Throws an exception if path is not a directory.
   Database(Metadata const&, bool overwrite = false);
   ~Database();
   inline Metadata const& metadata() const;
   inline Cache<File>& filesCache() const;
   inline Cache<Page>& pagesCache() const;
private:
   Database(Database const&);
   Database& operator =(Database const&);
private:
   Metadata m_meta;
   // Pointers are just to allow incomplete types
   Cache<File>* m_filesCache;
   Cache<Page>* m_pagesCache;
};

#include "database_impl.h"
