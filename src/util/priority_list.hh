
#ifndef UTIL_PRIORITY_LIST_HH
#define UTIL_PRIORITY_LIST_HH 1

#include <algorithm>  // std::lower_bound
#include <list>       // std::list
#include <functional> // std::less, std::equal_to

template<class V, class C = std::less<V>, class E = std::equal_to<V> >
class priority_list {
private:
  typedef std::list<V> list_t;
  typedef typename list_t::iterator list_it;

public:
  typedef V value_type;

  priority_list()
    { }

  const value_type& top() const
    { return m_list.back(); }

  void pop()
    { m_list.pop_back(); }

  void push(const value_type& v);

  bool drop(const value_type& v);

  bool empty() const
    { return m_list.empty(); }

private:
  list_t m_list;
};

// ========================================================================

template<class V, class C, class E>
void priority_list<V,C,E>::push(const value_type& v)
{
  if (empty()) {
    m_list.push_back(v);
  } else {
    list_it i = std::lower_bound(m_list.begin(), m_list.end(), v, C());
    m_list.insert(i, v);
  }
}

// ------------------------------------------------------------------------

template<class V, class C, class E>
bool priority_list<V,C,E>::drop(const V& v)
{
  list_it i = std::find_if(m_list.begin(), m_list.end(), std::bind1st(E(), v));
  if (i == m_list.end())
    return false;

  m_list.erase(i);
  return true;
}

#endif // UTIL_PRIORITY_LIST_HH
