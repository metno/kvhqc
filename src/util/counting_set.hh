
#ifndef UTIL_COUNTING_SET_HH
#define UTIL_COUNTING_SET_HH 1

#include <map>

template<class V, class C = std::less<V> >
class counting_set {
private:
  typedef std::map<V, size_t, C> map_t;

public:
  typedef typename map_t::value_type      value_type;
  typedef typename map_t::reference       reference;
  typedef typename map_t::const_reference const_reference;
  typedef typename map_t::pointer         pointer;
  typedef typename map_t::difference_type difference_type;
  typedef typename map_t::size_type       size_type;

  typedef typename map_t::iterator        iterator;
  typedef typename map_t::const_iterator  const_iterator;

  counting_set()
    { }

  iterator begin()
    { return iterator(m_map.begin()); }

  iterator end()
    { return iterator(m_map.end()); }

  const_iterator begin() const
    { return m_map.begin(); }

  const_iterator end() const
    { return m_map.end(); }

  bool empty() const
    { return m_map.empty(); }

  size_t count(const V& v) const;
  bool increase(const V& v);
  bool decrease(const V& v);

private:
  map_t m_map;
};

// ========================================================================

template<class V, class C>
size_t counting_set<V,C>::count(const V& v) const
{
  typename map_t::const_iterator it = m_map.find(v);
  if (it != m_map.end())
    return it->second;
  else
    return 0;
}

// ------------------------------------------------------------------------

template<class V, class C>
bool counting_set<V,C>::increase(const V& v)
{
  typename map_t::iterator it = m_map.find(v);
  if (it == m_map.end()) {
    m_map.insert(std::make_pair(v, 1));
    return true;
  } else {
    it->second += 1;
    return false;
  }
}

// ------------------------------------------------------------------------

template<class V, class C>
bool counting_set<V,C>::decrease(const V& v)
{
  typename map_t::iterator it = m_map.find(v);
  if (it == m_map.end())
    return false;
  if (it->second > 1) {
    it->second -= 1;
    return false;
  } else {
    m_map.erase(it);
    return true;
  }
}

#endif // UTIL_COUNTING_SET_HH
