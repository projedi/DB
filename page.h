#pragma once

#include <memory>

#include "database.h"
#include "cache.h"

struct Page: Cacheable<Page> {
   Page(Database const&, std::string const& name, uint64_t number);
   inline bool operator ==(Page const&) const;
   inline bool operator <(Page const&) const;
   inline std::string const& name() const;
   inline uint64_t number() const;
   template<class T> bool isValid(uint64_t offset) const;
   template<class T> void at(uint64_t& offset, T const&);
   template<class T> T at(uint64_t& offset) const;
   void purge();
private:
   friend struct File;
   // Just for File. This const modifier is a hack to allow File to load const Page.
   uint8_t*& buffer() const;
   void loadPage() const;
   void savePage() const;
private:
   Database const* m_db;
   std::string m_name;
   uint64_t m_number;
   struct Data;
   std::shared_ptr<Data> m_data;
};

#include "page_impl.h"
