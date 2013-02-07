#pragma once

#include <functional>

#include "index.h"

struct Hash: Index {
   Hash(Database const*, std::string const&, bool,
         std::vector<std::pair<Column, Direction>> const&);
   Hash(Database const*, std::string const&);
   bool remove(rowiterator const&) const;
   bool insert(rowcount_t, std::map<Column, void*> const&) const;
   rowiterator rowIterator(std::map<Column, std::vector<Predicate>> const&) const;
private:
   pagesize_t m_recordSize;
};
