#pragma once

#include "table.h"

Page* getPage(Database&, Table const&, rowcount_t row, pagesize_t& pageOffset);

// Does not check if such columns exist
void selectAll(Database&, std::ostream&, Table const&,
      std::vector<std::string> const& cols = std::vector<std::string>());

// From table.h:
// typedef std::pair<std::string, SqlType*> InputColumn;
inline void createTable(Database&, std::string const&, std::vector<InputColumn> const&);

// Doesn't check if such columns exist or if it typechecks
void insertInto(Database&, Table&, std::map<std::string, std::string> const&);

#include "cmdlist_impl.h"
