void createTable(Database& db, std::string const& name,
      std::vector<InputColumn> const& cols) { Table t(db, name, cols); }
