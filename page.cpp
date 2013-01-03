#include "page.h"
#include "file.h"

using std::string;

struct Page::Data {
   Data(Database const& db, string filename):
      m_file(db, filename), m_dirty(false), m_buffer(nullptr) { }
   File m_file;
   bool m_dirty;
   uint8_t* m_buffer;
   // To refrain from using lambdas
   struct Deleter {
      Deleter(Database const& db, Page const& p): m_db(db), m_page(p) { }
      void operator()(Page::Data* d) {
         if(d->m_buffer) m_db.pagesCache().remove(m_page);
         d->m_file.removePage(m_page);
         delete d;
      }
   private:
      Database const& m_db;
      Page const& m_page;
   };
};


Page::Page(Database const& db, string const& name, size_t number):
   m_db(&db), m_name(name), m_number(number) {
   auto res = m_db->pagesCache().find(*this);
   if(res) m_data = res->m_data;
   else {
      string filename = m_db->metadata().path + "/" + m_name;
      m_data.reset(new Data(*m_db, filename), Data::Deleter(*m_db, *this));
      m_data->m_file.addPage(*this);
   }
}

void Page::purge() {
   if(!m_data->m_buffer) return;
   savePage();
   m_data->m_file.releasePage(*this);
   m_data->m_buffer = 0;
   m_data->m_dirty = false;
}

void Page::loadPage() const {
   if(m_data->m_buffer) return;
   m_db->pagesCache().insert(*this);
   m_data->m_file.loadPage(*this);
   m_data->m_dirty = false;
}

void Page::savePage() const {
   if(!m_data->m_buffer || !m_data->m_dirty) return;
   m_data->m_file.savePage(*this);
   m_data->m_dirty = false;
}

uint8_t*& Page::buffer() const {
   m_data->m_dirty = true;
   return m_data->m_buffer;
}
