#pragma once

#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <fstream>
#include <sstream>

#include "database.h"
#include "page.h"
#include "types.h"

// TODO: Adding and removing indexes
struct Column {
   inline Column(std::string const&, size_t offset, SqlType const*);
   inline std::string const& name() const;
   inline size_t offset() const;
   inline size_t& offset();
   inline size_t size() const;
   inline size_t sizeHeader() const;
   friend void Page::at<Column>(uint64_t& offset, Column const&);
   friend Column Page::at<Column>(uint64_t& offset) const;
   inline void fromString(std::istream&, Page&, uint64_t& offset) const;
   inline void toString(std::ostream&, Page const&, uint64_t& offset) const;
private:
   Column() { }
private:
   std::shared_ptr<const SqlType> m_type;
   std::string m_name;
   size_t m_offset;
};

typedef std::pair<std::string, SqlType*> InputColumn;

// Has const_iterator over columns
// TODO: Benchmark std::vector against std::array
struct Table {
   typedef std::vector<Column>::const_iterator const_iterator;
   // Always creats new table without checking if it already exists
   inline Table(Database const&, std::string const&, std::vector<InputColumn> const& cols);
   inline ~Table();
   inline std::string const& name() const;
   inline size_t& rowCount();
   inline size_t rowCount() const;
   inline size_t rowSize() const;
   inline size_t headerSize() const;
   inline Page const& page() const;
   inline boost::optional<Column> find(std::string const&) const;
   inline const_iterator begin() const;
   inline const_iterator end() const;
   inline Column const& operator[](int) const;
   // Why isn't it const you say? Because page would become const
   inline void saveHeader();
   inline void loadHeader();
   static boost::optional<Table> findTable(Database const&, std::string const&);
private:
   inline Table(Database const&, std::string const&);
private:
   Page m_page;
   std::string m_name;
   std::vector<Column> m_cols;
   uint64_t m_rowCount;
   uint64_t m_rowSize;
   uint64_t m_headerSize;
};

#include "table_impl.h"
