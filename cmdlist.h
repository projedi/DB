#pragma once

#include "types.h"
#include "metadata.h"

#include <string>
#include <map>
#include <vector>
#include <ostream>

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::ostream;

int selectAll(std::ostream&, string const&, vector<string> const& cols = vector<string>());

inline void createTable(string const& name, vector<pair<string, SqlType*>> const& cols) {
   Metadata::instance().insert(Table(name, cols));
   Metadata::instance().flush();
}

int insertInto(string const& name, map<string, string> const& values);
