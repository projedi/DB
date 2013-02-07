#pragma once

#include "index.h"

// Getting Column from string:
//    boost::optional<Column> Table::find(std::string const&)

// If you think any of those typecheck values against types - think backwards.

// From types.h:
/* struct Predicate {
   enum OPER { EQ, NEQ, LT, GT, LEQ, GEQ };
   template<class T> Predicate(OPER, T*);
   OPER op;
   void* val;
}; */

// From table.h:
// typedef std::pair<std::string, SqlType*> InputColumn;

// TODO: Be dramatic when using unique indexes.

typedef std::vector<Column> Columns;
typedef std::map<Column, std::vector<Predicate>> Constraints;
typedef std::map<Column, void*> Values;
typedef std::vector<std::pair<Column, Index::Direction>> IndexColumns;

inline void selectAll(Database const*, std::ostream&, Table const&,
      Columns const& cols = Columns());

void selectWhere(Database const*, std::ostream&, Table const&, Constraints const&,
      Columns cols = Columns());

// TODO: Check if requested table exists and do the only sensible thing(except exit(1)).
inline void createTable(Database const*, std::string const&, std::vector<InputColumn> const&);

void insertInto(Database const*, Table&, Values const&);

void updateWhere(Database const*, Table&, Constraints const&, Values const&);

void deleteWhere(Database const*, Table&, Constraints const&);

void createIndex(Database const*, Table&, Index::Type, bool, IndexColumns const&);

#include "cmdlist_impl.h"
