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

template<class T>
struct Cache {
   // Cool, isn't? Emulating Haskell-like parametric polymorphism with constraints
   static_assert(std::is_base_of<Cacheable<T>, T>::value,
         "T must be derived from Cacheable<T>");
   Cache(size_t maxSize);
   ~Cache();
   boost::optional<T> find(T const&);
   void insert(T const&);
   void remove(T const&);
private:
   Cache(Cache<T> const&);
   Cache<T>& operator =(Cache<T> const&);
private:
   size_t m_maxSize;
   std::map<T, typename std::list<T>::iterator> m_map;
   std::list<T> m_list;
};

#include "cache_impl.h"
