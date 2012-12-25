#pragma once

#include <string>
#include <deque>

using std::string;

struct Page {
   Page(): m_name(""), m_number(0), m_refCount(0), m_buffer(0) { }
   Page(string const& name, size_t number);
   ~Page();
   Page(Page const&);
   friend void swap(Page&,Page&);
   Page& operator =(Page const& src);
   // If you free a returned pointer, everything will die the most gruesome death
   uint8_t* operator +(size_t shift);
   bool operator ==(Page const& p) { return m_name == p.m_name && m_number == p.m_number; }
   bool operator !=(Page const& p) { return !(*this == p); }
   void flush() const { savePage(); }
   string const& name() const { return m_name; }
   size_t number() const { return m_number; }
   friend struct PageManager;
private:
   void loadPage();
   void savePage() const;
   void purge();
private:
   string m_name;
   size_t m_number;
   uint64_t* m_refCount;
   uint8_t** m_buffer;
};

// TODO: What to do when on low memory interpage data dependencies cause deadlocks?
// TODO: Removing pages from disk.
struct PageManager {
   static void create(string const& directory, size_t maxPageCount, size_t pageSize)
      { m_instance = new PageManager(directory, maxPageCount, pageSize); }
   static void destroy() { delete m_instance; }
   static PageManager& instance() { return *m_instance; }
   size_t pageSize() const { return m_pageSize; }
   size_t maxPageCount() const { return m_maxPageCount; }
   friend struct Page;
private:
   PageManager(string const& directory, size_t maxPageCount, size_t pageSize):
      m_directory(directory), m_maxPageCount(maxPageCount), m_pageSize(pageSize) { }
   ~PageManager() {
      for(auto page: m_cache)
         page.purge();
   }
   // TODO: Make it faster
   Page getPage(string const& name, size_t number);
   void loadPage(Page&);
   void savePage(Page const&);
   PageManager(PageManager const&);
   PageManager& operator=(PageManager const&);
private:
   static PageManager* m_instance;
   string m_directory;
   std::deque<Page> m_cache;
   size_t m_maxPageCount;
   size_t m_pageSize;
};

