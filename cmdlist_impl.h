void createTable(Database const& db, std::string const& name,
      std::vector<InputColumn> const& cols) { Table t(db, name, cols); }
