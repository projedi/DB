#pragma once

#include "table.h"

#include <string>
#include <map>
#include <vector>
#include <ostream>

Page* getPage(Database const&, Table const&, size_t row, size_t& pageOffset);

// Does not check if such columns exist
int selectAll(Database const&, std::ostream&, Table const&,
      std::vector<std::string> const& cols = std::vector<std::string>());

// From table.h:
// typedef std::pair<std::string, SqlType*> InputColumn;
inline void createTable(Database const&, std::string const&, std::vector<InputColumn> const&);

// Doesn't check if such columns exist or if it typechecks
void insertInto(Database const&, Table&, std::map<std::string, std::string> const&);

#include "cmdlist_impl.h"
