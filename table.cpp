#include "table.h"

boost::optional<Table> Table::findTable(Database const& db, std::string const& name) {
   std::fstream f(db.metadata().path + "/" + "table-" + name, std::fstream::in);
   if(f.is_open()) return boost::optional<Table>(Table(db, name));
   else return boost::optional<Table>();
}
