#pragma once

#include "table.h"

struct Index;

struct rowiterator {
   typedef std::function<boost::optional<std::pair<rowcount_t,pagesize_t>> (rowiterator const&)> nextf;
   //typedef boost::optional<std::pair<rowcount_t,pagesize_t>> (*nextf) (rowiterator const&);
   inline rowiterator(std::shared_ptr<Index const>, nextf, std::pair<rowcount_t,pagesize_t>);
   inline rowiterator& operator ++();
   inline operator bool() const;
   inline rowcount_t operator *() const;
   inline rowcount_t row() const;
   inline pagesize_t offset() const;
   inline std::shared_ptr<Index const> owner() const;
   inline bool active() const;
private:
   rowcount_t m_row;
   pagesize_t m_offset;
   std::shared_ptr<Index const> m_owner;
   nextf m_next;
   bool m_active;
};

struct Column;
struct Table;

// TODO: Must be designed so iterators won't break on remove of current row
struct Index {
   enum Type { Hash, BTree };
   enum Direction { ASC, DESC };
   virtual inline ~Index();
   inline bool unique() const;
   inline Type type() const;
   inline std::vector<std::pair<Column, Direction>> const& cols() const;
   void saveHeader() const;
   void loadHeader();
   // const logic is like this: header didn't change - nothing changed
   virtual bool remove(rowiterator const&) const = 0;
   virtual bool insert(rowcount_t, std::map<Column, void*> const& vals) const = 0;
   virtual rowiterator rowIterator(
      std::map<Column, std::vector<Predicate>> const&) const = 0;
   inline static std::shared_ptr<Index> findIndex(Database const*, std::string const&);
   inline static std::string getAvailableName(Database const*, Table const&);
   static std::shared_ptr<Index> createIndex(Database const*, Table const&, Index::Type, bool isUnique, std::vector<std::pair<Column, Direction>> const&);
protected:
   Index(Database const*, std::string const&, bool, Type type,
         std::vector<std::pair<Column, Direction>> const&);
   Index(Database const*, std::string const&);
   static std::shared_ptr<Index> openIndex(Database const*, std::string const&);
private:
   Index(Index const&);
   Index& operator=(Index const&);
protected:
   mutable Page m_page;
   Type m_type;
   bool m_unique;
   std::vector<std::pair<Column, Direction>> m_cols;
   pagesize_t m_headerSize;
};

#include "index_impl.h"
