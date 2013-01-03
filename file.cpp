#include <set>

#include "file.h"
#include "page.h"

// It is broken now. Might be benchmarked against later
#ifdef USE_MMAP
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

using std::set;
using std::string;

struct File::Data {
   Data(): m_file(0) { }
#ifdef USE_MMAP
   int m_file;
   set<Page> m_pages;
#else
   FILE* m_file;
#endif
   // To refrain from using lambdas
   struct Deleter {
      Deleter(Database const& db, File const& f): m_db(db), m_file(f) { }
      void operator()(File::Data* d) {
         if(d->m_file) m_db.filesCache().remove(m_file);
         delete d;
      }
   private:
      Database const& m_db;
      File const& m_file;
   };
};

File::File(Database const& db, string const& name): m_db(db), m_name(name) {
   auto res = m_db.filesCache().find(*this);
   if(res) m_data = res->m_data;
   else m_data.reset(new Data(), Data::Deleter(m_db, *this));
}

void File::addPage(Page const& page) {
#ifdef USE_MMAP
   m_data->m_pages.insert(page);
#endif
}

void File::removePage(Page const& page) {
#ifdef USE_MMAP
   m_data->m_pages.erase(page);
#endif
}

void File::releasePage(Page& page) {
#ifdef USE_MMAP
   //TODO: check for errors
   munmap(page.buffer(), m_db.metadata().pageSize);
#else
   delete [] page.buffer();
#endif
}

void File::loadPage(Page const& page) {
   if(!m_data->m_file) loadFile();
   size_t pageSize = m_db.metadata().pageSize;
#ifdef USE_MMAP
   // TODO: Check if I really need any mapping done
   //uint8_t* addr = new uint8_t[pageSize];
   // TODO: Check for errors here
   size_t wantedSize = page.number() * pageSize + pageSize;
   struct stat fileStat;
   fstat(m_data->m_file, &fileStat);
   if(fileStat.st_size < wantedSize) ftruncate(m_data->m_file, wantedSize);
   uint8_t* res = (uint8_t*) mmap(nullptr, pageSize, PROT_READ | PROT_WRITE,
         MAP_SHARED, m_data->m_file, page.number() * pageSize);
#else
   uint8_t* res = new uint8_t[pageSize];
   // TODO: Check for errors here and zero out on EOF
   fseek(m_data->m_file, 0, SEEK_END);
   long end = ftell(m_data->m_file);
   fseek(m_data->m_file, page.number() * pageSize, SEEK_SET);
   long cur = ftell(m_data->m_file);
   if(cur < end) fread(res, 1, pageSize, m_data->m_file);
   else {
      //for(int i = 0; i != pageSize; ++i) res[i] = 0;
      for(uint64_t i = 0; i != pageSize; ++i) res[i] = 0xe;
   }
#endif
   page.buffer() = res;
}

void File::savePage(Page const& page) {
   size_t pageSize = m_db.metadata().pageSize;
#ifdef USE_MMAP
   //TODO: Check if i need it at all
   msync(page.buffer(), page.number() * pageSize, MS_ASYNC);
#else
   if(!m_data->m_file) loadFile();
   fseek(m_data->m_file, page.number() * pageSize, SEEK_SET);
   fwrite(page.buffer(), 1, pageSize, m_data->m_file);
#endif
}

void File::purge() {
   if(!m_data->m_file) return;
#ifdef USE_MMAP
   auto pages = m_data->m_pages;
   for(auto page = pages.begin(); page != pages.end(); ++page)
      m_db.pagesCache().remove(*page);
   close(m_data->m_file);
   m_data->m_file = 0;
#else
   fclose(m_data->m_file);
   m_data->m_file = nullptr;
#endif
}

void File::loadFile() {
   if(m_data->m_file) return;
   m_db.filesCache().insert(*this);
#ifdef USE_MMAP
   int res = open(m_name.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
   m_data->m_file = res;
#else
   FILE* res = fopen(m_name.c_str(), "r+");
   if(!res) res = fopen(m_name.c_str(), "w+");
   m_data->m_file = res;
#endif
}
