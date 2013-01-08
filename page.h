#pragma once

#include <memory>

#include "cache.h"

struct Database;

typedef uint64_t pagenumber_t;
// TODO: change to uint32_t
typedef uint64_t pagesize_t;

struct Page: Cacheable<Page> {
   Page(Database&, std::string const&, pagenumber_t);
   // TODO: try to make them faster to help Cache::find out
   inline bool operator ==(Page const&) const;
   inline bool operator <(Page const&) const;
   inline std::string const& name() const;
   inline pagenumber_t number() const;
   // Implemented like that because sometimes it's
   // not enough to plainly map memory
   template<class T> void at(pagesize_t& offset, T const&);
   template<class T> T at(pagesize_t& offset) const;
   void purge();
private:
   void setDirty();
   // This const modifier is a hack for const version of 'at'
   uint8_t*& buffer() const;
   void loadPage() const;
   void savePage() const;
private:
   Database* m_db;
   std::string m_name;
   pagenumber_t m_number;
   struct Data;
   std::shared_ptr<Data> m_data;
};

#include "page_impl.h"
