#include "file.h"
#include "database.h"

using std::string;

File::File(Database& db, string const& name): m_db(&db), m_name(name) {
   auto res = m_db->filesCache().find(*this);
   if(res) m_file = res->m_file;
   else m_file.reset(new FILE*(nullptr));
}

File::~File() {
   if(m_file.unique() && *m_file)
      m_db->filesCache().remove(*this);
}

uint8_t* File::loadPage(pagenumber_t number) {
   if(!*m_file) loadFile();
   pagesize_t pageSize = m_db->metadata().pageSize;
   uint8_t* buffer = new uint8_t[pageSize];
   FILE* file = *m_file;
   long needLen = (number + 1) * pageSize;
   fseek(file, 0, SEEK_END);
   long fileLen = ftell(file);
   if(needLen <= fileLen) {
      fseek(file, number * pageSize, SEEK_SET);
      fread(buffer, pageSize, 1, file);
   } else
      for(pagesize_t i = 0; i != pageSize; ++i) buffer[i] = EMPTY_BYTE;
   return buffer;
}

void File::savePage(uint8_t* buffer, pagenumber_t number) {
   if(!*m_file) loadFile();
   pagesize_t pageSize = m_db->metadata().pageSize;
   FILE* file = *m_file;
   fseek(file, number * pageSize, SEEK_SET);
   fwrite(buffer, pageSize, 1, file);
}

void File::purge() {
   if(!*m_file) return;
   fclose(*m_file);
   *m_file = nullptr;
}

void File::loadFile() {
   m_db->filesCache().insert(*this);
   FILE* file = fopen(m_name.c_str(), "r+");
   if(!file) file = fopen(m_name.c_str(), "w+");
   *m_file = file;
}
