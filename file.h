#pragma once

#include "page.h"

//const uint8_t EMPTY_BYTE = 0;
const uint8_t EMPTY_BYTE = 0xe;

struct File: Cacheable<File> {
   File(Database&, std::string const&);
   ~File();
   inline bool operator ==(File const&) const;
   inline bool operator <(File const&) const;
   inline std::string const& name() const;
   uint8_t* loadPage(pagenumber_t);
   void savePage(uint8_t*, pagenumber_t);
   void purge();
private:
   void loadFile();
private:
   Database* m_db;
   std::string m_name;
   std::shared_ptr<FILE*> m_file;
};

#include "file_impl.h"
