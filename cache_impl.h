template<class T>
bool Cacheable<T>::operator !=(T const& o) const { return !(*this == o); }
template<class T>
bool Cacheable<T>::operator >(T const& o) const { return o < *this; }
template<class T>
bool Cacheable<T>::operator <=(T const& o) const { return !(*this > o); }
template<class T>
bool Cacheable<T>::operator >=(T const& o) const { return !(*this < o); }

template<class T>
Cache<T>::Cache(size_t maxSize): m_maxSize(maxSize) { }

template<class T>
Cache<T>::~Cache() {
   for(auto elem = m_list.begin(); elem != m_list.end(); ++elem)
      elem->purge();
}

template<class T>
boost::optional<T> Cache<T>::find(T const& val) {
   auto mapIt = m_map.find(val);
   if(mapIt == m_map.end()) return boost::optional<T>();
   T res = mapIt->first;
   // Optimisation to put most recently used at the top
   m_list.erase(mapIt->second);
   m_list.push_front(res);
   mapIt->second = m_list.begin();
   return boost::optional<T>(res);
}

template<class T>
void Cache<T>::insert(T const& val) {
   if(m_list.size() == m_maxSize) remove(m_list.back());
   m_list.push_front(val);
   m_map.insert(make_pair(val, m_list.begin()));
}

template<class T>
void Cache<T>::remove(T const& val) {
   auto mapIt = m_map.find(val);
   if(mapIt == m_map.end()) return;
   T res = mapIt->first;
   m_list.erase(mapIt->second);
   m_map.erase(mapIt);
   res.purge();
}
