
#ifndef UTIL_LRU_CACHE_HH
#define UTIL_LRU_CACHE_HH 1

#include <list>
#include <unordered_map>
#include <cassert>

template <class K, class V, class KH = std::hash<K>, class KE = std::equal_to<K> >
class lru_cache
{
private:
  typedef std::list< std::pair<K, V> > items_t;
  typedef typename items_t::iterator items_it;
  typedef std::unordered_map<K, items_it, KH, KE> keys_t;

public:
  lru_cache(size_t cache_size)
    : cache_size_(cache_size) { assert(cache_size>0); }

  void put(const K &key, const V &value);

  bool has(const K &key) const
    { return (keys_.find(key) != keys_.end()); }

  const V& get(const K &key)
    {
      auto kit = keys_.find(key);
      assert(kit != keys_.end());
      items_.splice(items_.begin(), items_, kit->second);
      return items_.front().second;
    }

private:
  items_t items_;
  keys_t keys_;
  size_t cache_size_;
};

template <class K, class V, class KH, class KE>
void lru_cache<K, V, KH, KE>::put(const K &key, const V &value)
{
  auto kit = keys_.find(key);
  if (kit != keys_.end()) {
    items_.splice(items_.begin(), items_, kit->second);
    items_.front().second = value;
  } else {
    items_.push_front(std::make_pair(key, value));
    keys_.insert(std::make_pair(key, items_.begin()));
    if (items_.size() > cache_size_) {
      keys_.erase(items_.back().first);
      items_.pop_back();
    }
  }
}

#endif // UTIL_LRU_CACHE_HH
