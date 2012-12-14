#pragma once

#include <string>
#include <deque>

using std::string;
using std::deque;

struct Page {
   Page(string const& name, size_t number, uint8_t* buf, string const& m_filename);
   ~Page();
   Page(Page const&);
   // TODO: Check self assignment
   Page& operator =(Page const&);
   uint8_t& operator *() { return **m_buffer; }
   uint8_t& operator [](size_t idx) { return (*m_buffer)[idx]; }
   void setBuffer(uint8_t* buf) { *m_buffer = 0; }
   void purge();
   void flush();
   void read();
   string const& name() const { return m_name; }
   size_t number() const { return m_number; }
   friend void swap(Page&,Page&);
private:
   // TODO: Should it be private?
   void askForSpace();
private:
   string m_name;
   size_t m_number;
   uint64_t* m_refCount;
   uint8_t** m_buffer;
   string m_filename;
};

// TODO: What to do when on low memory interpage data dependencies cause deadlocks?
// TODO: Removing pages from disk.
// TODO: Faster search for page in cache
struct PageManager {
   static void create(string const& directory, size_t maxPageCount, size_t pageSize)
      { m_instance = new PageManager(directory, maxPageCount, pageSize); }
   static void destroy() { delete m_instance; }
   static PageManager& instance() { return *m_instance; }
   Page getPage(string const& name, size_t number);
   // Only when page is not in cache
   Page getPage(Page& page);
   size_t pageSize() const { return m_pageSize; }
private:
   PageManager(string const& directory, size_t maxPageCount, size_t pageSize):
      m_directory(directory), m_maxPageCount(maxPageCount), m_pageSize(pageSize) { }
   ~PageManager() {
      for(auto page: m_cache)
         page->purge();
   }
   PageManager(PageManager const&);
   PageManager& operator=(PageManager const&);
private:
   static PageManager* m_instance;
   string m_directory;
   deque<Page> m_cache;
   size_t m_maxPageCount;
   size_t m_pageSize;
};
