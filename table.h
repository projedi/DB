#pragma once

#include <vector>
#include <fstream>

#include "types.h"
#include "database.h"

struct Column {
   inline Column(std::string const&, pagesize_t offset, SqlType const*);
   inline std::string const& name() const;
   inline pagesize_t offset() const;
   inline pagesize_t& offset();
   inline SqlType const& type() const;
   inline pagesize_t size() const;
   inline pagesize_t sizeHeader() const;
   inline bool operator <(Column const&) const;
   friend void Page::at<Column>(pagesize_t& offset, Column const&);
   friend Column Page::at<Column>(pagesize_t& offset) const;
private:
   std::shared_ptr<SqlType const> m_type;
   std::string m_name;
   pagesize_t m_offset;
};

typedef uint64_t rowcount_t;
typedef uint8_t colcount_t;

typedef std::pair<std::string, SqlType*> InputColumn;

struct Index;
struct rowiterator;

// TODO: Benchmark std::vector against std::array
// TODO: Introduce dirty state to know when to save a header
struct Table {
   // Always creats new table without checking if it already exists
   inline Table(Database const*, std::string const&, std::vector<InputColumn> const& cols);
   inline ~Table();
   inline std::string const& name() const;
   inline rowcount_t& rowCount();
   inline rowcount_t rowCount() const;
   inline pagesize_t rowSize() const;
   inline pagesize_t headerSize() const;
   inline Page const& page() const;
   inline boost::optional<Column> findColumn(std::string const&) const;
   inline std::vector<Column> const& cols() const;
   inline void addIndex(std::shared_ptr<Index const>);
   inline std::vector<std::shared_ptr<Index const>> const& indexes() const;
   rowiterator rowIterator() const;
   inline Page* getPage(rowcount_t, pagesize_t&) const;
   inline static boost::optional<Table> findTable(Database const*, std::string const&);
   static uint8_t const ADD_FLAG = 0xaa;
   static uint8_t const DEL_FLAG = 0xee;
private:
   inline Table(Database const*, std::string const&);
   void loadIndexes();
   inline void saveHeader() const;
   inline void loadHeader();
private:
   mutable Page m_page;
   std::string m_name;
   std::vector<Column> m_cols;
   std::vector<std::shared_ptr<Index const>> m_indexes;
   rowcount_t m_rowCount;
   pagesize_t m_rowSize;
   pagesize_t m_headerSize;
};

#include "table_impl.h"
