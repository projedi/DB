#pragma once

#include <vector>
//#include <boost/optional.hpp>
#include <fstream>
#include <sstream>

#include "types.h"
#include "database.h"

// TODO: Adding and removing indexes
struct Column {
   inline Column(std::string const&, pagesize_t offset, SqlType const*);
   inline std::string const& name() const;
   inline pagesize_t offset() const;
   inline pagesize_t& offset();
   inline SqlType const& type() const;
   inline pagesize_t size() const;
   inline pagesize_t sizeHeader() const;
   friend void Page::at<Column>(pagesize_t& offset, Column const&);
   friend Column Page::at<Column>(pagesize_t& offset) const;
   inline void fromString(std::istream&, Page&, pagesize_t& offset) const;
   inline void toString(std::ostream&, Page const&, pagesize_t& offset) const;
private:
   std::shared_ptr<SqlType const> m_type;
   std::string m_name;
   pagesize_t m_offset;
};

typedef std::pair<std::string, SqlType*> InputColumn;

typedef uint64_t rowcount_t;
typedef uint8_t colcount_t;

// Has const_iterator over columns
// TODO: Benchmark std::vector against std::array
// TODO: Introduce dirty state
struct Table {
   typedef std::vector<Column>::const_iterator const_iterator;
   // Always creats new table without checking if it already exists
   inline Table(Database&, std::string const&, std::vector<InputColumn> const& cols);
   inline ~Table();
   inline std::string const& name() const;
   inline rowcount_t& rowCount();
   inline rowcount_t rowCount() const;
   inline pagesize_t rowSize() const;
   inline pagesize_t headerSize() const;
   inline Page const& page() const;
   inline boost::optional<Column> find(std::string const&) const;
   inline const_iterator begin() const;
   inline const_iterator end() const;
   inline Column const& operator[](colcount_t) const;
   // Why isn't it const you say? Because page would become const
   inline void saveHeader();
   inline void loadHeader();
   inline static boost::optional<Table> findTable(Database&, std::string const&);
private:
   inline Table(Database&, std::string const&);
private:
   Page m_page;
   std::string m_name;
   std::vector<Column> m_cols;
   rowcount_t m_rowCount;
   pagesize_t m_rowSize;
   pagesize_t m_headerSize;
};

#include "table_impl.h"
