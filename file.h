#pragma once

#include <memory>

#include "database.h"
#include "cache.h"

struct Page;

struct File: Cacheable<File> {
   File(Database const&, std::string const& name);
   inline bool operator ==(File const&) const;
   inline bool operator <(File const&) const;
   inline std::string const& name() const;
   void addPage(Page const&);
   void removePage(Page const&);
   void releasePage(Page&);
   void loadPage(Page const&);
   void savePage(Page const&);
   uint8_t* loadPage(size_t number);
   void savePage(uint8_t*, size_t number);
   void purge();
private:
   void loadFile();
private:
   Database const& m_db;
   std::string m_name;
   struct Data;
   std::shared_ptr<Data> m_data;
};

#include "file_impl.h"
