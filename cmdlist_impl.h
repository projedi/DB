void selectAll(Database const* db, std::ostream& ost, Table const& table,
      Columns const& cols) {
   selectWhere(db, ost, table, Constraints(), cols);
}

void createTable(Database const* db, std::string const& name,
      std::vector<InputColumn> const& cols) { Table t(db, name, cols); }
