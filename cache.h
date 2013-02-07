#pragma once

#include <type_traits>
#include <boost/optional.hpp>
#include <map>
#include <list>

template<class T>
struct Cacheable {
   virtual ~Cacheable() { }
   virtual void purge() = 0;
   virtual bool operator ==(T const&) const = 0;
   virtual bool operator <(T const&) const = 0;
   bool operator !=(T const&) const;
   bool operator >(T const&) const;
   bool operator <=(T const&) const;
   bool operator >=(T const&) const;
};

typedef uint64_t cachesize_t;

template<class T>
struct Cache {
   // Cool, isn't? Emulating Haskell-like parametric polymorphism with constraints
   static_assert(std::is_base_of<Cacheable<T>, T>::value,
         "Cache<T>: T must be derived from Cacheable<T>");
   Cache(cachesize_t maxSize);
   ~Cache();
   boost::optional<T> find(T const&) const;
   // Logically const is a bendable definition, you know.
   void insert(T const&) const;
   void remove(T const&) const;
private:
   Cache(Cache<T> const&);
   Cache<T>& operator =(Cache<T> const&);
private:
   cachesize_t m_maxSize;
   mutable std::map<T, typename std::list<T>::iterator> m_map;
   mutable std::list<T> m_list;
};

#include "cache_impl.h"
