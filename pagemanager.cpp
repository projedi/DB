#include "pagemanager.h"

#include <fstream>
#include <sstream>
#include <algorithm>

using std::fstream;
using std::deque;

Page::Page(string const& name, size_t number, uint8_t* buf, string const& filename):
   m_name(name), m_number(number), m_refCount(new int64_t(1)), m_buffer(new int*(buf)),
   m_filename(filename) { }

Page::~Page() {
   --(*m_refCount);
   if(!*m_refCount) {
      purge();
      delete m_refCount;
      delete m_buffer;
   }
}

Page::Page(Page const& p):
   m_name(p.m_name), m_number(p.m_number), m_refCount(p.m_refCount), m_buffer(p.m_buffer),
   m_filename(p.m_filename) {
   ++(*m_refCount);
}

void swap(Page& p1, Page& p2) {
   std::swap(p1.m_name, p2.m_name);
   std::swap(p1.m_number, p2.m_number);
   std::swap(p1.m_refCount, p2.m_refCount);
   std::swap(p1.m_buffer, p2.m_buffer);
}

Page& operator =(Page const& src) {
   Page copy = src;
   swap(*this, copy);
   return *this;
}

void Page::purge() {
   if(!*m_buffer) return;
   flush();
   delete [] *m_buffer; 
   *m_buffer = 0;
}

// TODO: Not so silent fail
void Page::flush() {
   if(!*m_buffer) return;
   fstream file(name, fstream::out);
   size_t pageSize = (PageManager::instance()).pageSize();
   file.seekp(number * pageSize);
   file.write(*m_buffer, pageSize);
}

// TODO: Not so silent fail
void Page::read() {
   if(!*m_buffer) return;
   fstream file(name, fstream::in);
   size_t pageSize = (PageManager::instance()).pageSize();
   file.seekg(number * pageSize);
   file.read(*m_buffer, pageSize);
}

void Page::askForSpace() {
   if(*m_buffer) return;
   (PageManager::instance()).getPage(*this);
}

Page getPage(string const& name, size_t number) {
   for(auto page: m_cache) {
      if(page->name() == name && page->number() == number) {
         Page res = *page;
         m_cache.erase(page);
         m_cache.push_front(res);
         return res;
      }
   }
   if(m_maxPageCount == m_cache.size()) {
      m_cache.back().purge();
      m_cache.pop_back();
   }
   uint8_t* buffer = new uint8_t[m_pageSize];
   Page newPage(name, number, buffer, directory + "/" + name);
   newPage.read();
   m_cache.push_front(newPage);
   return newPage;
}

Page getPage(Page& page) {
   if(m_maxPageCount == m_cache.size()) {
      m_cache.back().purge();
      m_cache.pop_back();
   }
   uint8_t* buffer = new uint8_t[m_pageSize];
   page.setBuffer(buffer);
   m_cache.push_front(page);
   return page;
}
