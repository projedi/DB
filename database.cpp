#include <boost/filesystem.hpp>
#include <fstream>

#include "database.h"

using namespace boost::filesystem;
using std::fstream;
using std::string;

// TODO: Overwrite should really clean everything up in a directory
void synchroniseDB(string const& dirPath, bool overwrite, pagesize_t& pageSize) {
   fstream fs;
   path dir(dirPath);
   path dbFile = dir;
   dbFile /= "db";
   if(exists(dbFile)) {
      if(overwrite) {
         // TODO: Clean everything in a directory
         fs.open(dbFile.string(), fstream::out);
      } else {
         fs.open(dbFile.string(), fstream::in);
         fs >> pageSize; 
         return;
      }
   } else if(exists(dir)) {
      if(is_directory(dir)) {
         // TODO: Clean everything in a directory
         fs.open(dbFile.string(), fstream::out);
      } else throw "I'm sorry but the provided path is not a directory at all";
   } else {
      // TODO: It should really be checked under Windows.
      create_directory(dir);
      fs.open(dbFile.string(), fstream::out);
   }
   fs << pageSize;
}

Database::Database(Metadata const& meta, bool overwrite):
   m_meta(meta), m_filesCache(meta.maxFilesCount), m_pagesCache(meta.maxPagesCount) {
   synchroniseDB(m_meta.path, overwrite, m_meta.pageSize);
}
