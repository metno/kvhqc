
#ifndef ACCESS_TAGGEDOBJECT_HH
#define ACCESS_TAGGEDOBJECT_HH 1

#include <boost/noncopyable.hpp>
#include <memory>

class TaggedObject : public std::enable_shared_from_this<TaggedObject>, boost::noncopyable
{
public:
  TaggedObject() : mTag(0) { }
  virtual ~TaggedObject() { }
  
  typedef void* Tag;

  Tag setTag(Tag tag)
    { std::swap(mTag, tag); return tag; }

  Tag tag() const
    { return mTag; }

private:
  Tag mTag;
};

#endif // ACCESS_TAGGEDOBJECT_HH
