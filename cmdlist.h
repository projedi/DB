#pragma once

#include "types.h"
#include "metadata.h"

#include <string>
#include <map>
#include <vector>

using std::string;
using std::vector;
using std::map;
using std::pair;

int selectAll(ostream&, string const& tablename);

inline void createTable(string const& name, vector<pair<string, SqlType*>> const& columns) {
   Metadata::instance().insert(Table(name, columns));
   Metadata::instance().flush();
}

int insertInto(string const& name, map<string, string> const& values);

int checkForColumns(string const& name, vector<string> const& columns);
int checkForColumns(string const& name, vector<pair<string, SqlType*>> const& columns);
