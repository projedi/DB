#include "pagemanager.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>

using std::fstream;
using std::deque;
using std::string;

PageManager* PageManager::m_instance = 0;

Page::Page(string const& name, size_t number):
   m_name(name), m_number(number), m_refCount(0), m_buffer(0) {
   Page res = PageManager::instance().getPage(name, number);
   if(res != Page()) swap(*this, res);
   else {
      m_refCount = new uint64_t(1);
      m_buffer = new uint8_t*(0);
   }
}

Page::~Page() {
   if(m_refCount) {
      --(*m_refCount);
      if(!*m_refCount) {
         purge();
         delete m_refCount;
         delete m_buffer;
      }
   }
}

Page::Page(Page const& src):
   m_name(src.m_name), m_number(src.m_number),
   m_refCount(src.m_refCount), m_buffer(src.m_buffer) {
      if(m_refCount) ++(*m_refCount);
}

void swap(Page& p1, Page& p2) {
   std::swap(p1.m_name, p2.m_name);
   std::swap(p1.m_number, p2.m_number);
   std::swap(p1.m_refCount, p2.m_refCount);
   std::swap(p1.m_buffer, p2.m_buffer);
}

Page& Page::operator =(Page const& src) {
   Page copy = src;
   swap(*this, copy);
   return *this;
}

uint8_t* Page::operator +(size_t shift) {
   if(!m_buffer) return 0;
   if(!*m_buffer) loadPage();
   return *m_buffer + shift;
}

void Page::loadPage() { PageManager::instance().loadPage(*this); }
void Page::savePage() const { PageManager::instance().savePage(*this); }

void Page::purge() {
   if(!m_buffer) return;
   if(!*m_buffer) return;
   savePage();
   delete [] *m_buffer;   
   *m_buffer = 0;
}

// TODO: Improve search efficiency
Page PageManager::getPage(string const& name, size_t number) {
   for(auto page = m_cache.begin(); page != m_cache.end(); ++page) {
      if(page->name() == name && page->number() == number) {
         Page res = *page;
         m_cache.push_front(res);
         m_cache.erase(page);
         return res;
      }
   }
   return Page();
}

void PageManager::loadPage(Page& page) {
   uint8_t* buffer = *(page.m_buffer);
   if(!buffer) {
      if(m_cache.size() == m_maxPageCount) {
         m_cache.back().purge();
         m_cache.pop_back();
      }
      m_cache.push_front(page);
      *(page.m_buffer) = new uint8_t[m_pageSize];
      buffer = *(page.m_buffer);
   }
   string filename = m_directory + "/" + page.m_name;
   size_t shift = page.m_number * m_pageSize;
   fstream file(filename, fstream::in | fstream::binary);  
   file.seekg(shift);
   file.read((char*)buffer, m_pageSize);
   file.close();
}

void PageManager::savePage(Page const& page) {
   uint8_t const* buffer = *(page.m_buffer);
   if(!buffer) return;
   string filename = m_directory + "/" + page.m_name;
   size_t shift = page.m_number * m_pageSize;
   fstream file(filename, fstream::in | fstream::out | fstream::binary);  
   file.seekp(shift);
   file.write((char*)buffer, m_pageSize);
   file.close();
}
