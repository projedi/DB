#include "file.h"
#include "database.h"

using std::string;

struct Page::Data {
   //Data(Database& db, string const& filename):
      //m_file(db, filename), m_dirty(false), m_buffer(nullptr) { }
   File m_file;
   bool m_dirty;
   uint8_t* m_buffer;
struct Deleter {
   Deleter(Database& db, Page const& p): m_db(db), m_page(p) { }
   void operator()(Page::Data* d) {
      if(d->m_buffer) m_db.pagesCache().remove(m_page);
      delete d;
   }
private:
   Database& m_db;
   Page const& m_page;
};
};

// To refrain from using lambdas

Page::Page(Database& db, string const& name, pagenumber_t number):
   m_db(&db), m_name(name), m_number(number) {
   auto res = m_db->pagesCache().find(*this);
   if(res) m_data = res->m_data;
   else {
      string filename = m_db->metadata().path + "/" + m_name;
      m_data.reset(new Data{File(*m_db, filename), false, nullptr},
            Data::Deleter(*m_db, *this));
   }
}

void Page::purge() {
   if(!m_data->m_buffer) return;
   savePage();
   delete [] m_data->m_buffer;
   m_data->m_buffer = nullptr;
   m_data->m_dirty = false;
}

void Page::setDirty() { m_data->m_dirty = true; }

uint8_t*& Page::buffer() const { return m_data->m_buffer; }

void Page::loadPage() const {
   if(m_data->m_buffer) return;
   m_db->pagesCache().insert(*this);
   m_data->m_buffer = m_data->m_file.loadPage(m_number);
   m_data->m_dirty = false;
}

void Page::savePage() const {
   if(!m_data->m_buffer || !m_data->m_dirty) return;
   m_data->m_file.savePage(m_data->m_buffer, m_number);
   m_data->m_dirty = false;
}
