#ifndef __SMART_IDMAP_V2_H__
#define __SMART_IDMAP_V2_H__

#include "common.h"
#include <map>
#include <arx/Memory.h>
#include <utility>

namespace smart {

  /**
   * A minimalistic map class, which indexes objects by globally unique 
   * identifiers. This map functions in a way that all objects that have ever
   * been put in it will have unique identifiers.
   *
   * TODO: arx::classnew_allocator fails here...
   */
  template<class Type, class Allocator = std::allocator<std::pair<const int, Type> > >
  class IdMap {
  public:
    typedef std::map<int, Type, std::less<int>, Allocator> map_type;

    typedef typename map_type::key_type key_type;
    typedef typename map_type::mapped_type mapped_type;
    typedef typename map_type::referent_type referent_type;
    typedef typename map_type::key_compare key_compare;
    typedef typename map_type::value_compare value_compare;
    typedef typename map_type::allocator_type allocator_type;
    typedef typename map_type::size_type size_type;
    typedef typename map_type::difference_type difference_type;
    typedef typename map_type::pointer pointer;
    typedef typename map_type::const_pointer const_pointer;
    typedef typename map_type::reference reference;
    typedef typename map_type::const_reference const_reference;
    typedef typename map_type::iterator iterator;
    typedef typename map_type::const_iterator const_iterator;
    typedef typename map_type::reverse_iterator reverse_iterator;
    typedef typename map_type::const_reverse_iterator const_reverse_iterator;
    typedef typename map_type::value_type value_type;

    IdMap(): mFirstFreeId(0) {}

    /** @returns the object with the given identifier id.
     * 
     * Note that it differs from a standard map in a sense that if such id does
     * not exist, then the operation will fail. */
    mapped_type& operator[] (key_type id) {
      map_type::iterator pos = mMap.find(id);
      assert(pos != mMap.end());
      return pos->second;
    }

    const mapped_type& operator[] (key_type id) const {
      map_type::const_iterator pos = mMap.find(id);
      assert(pos != mMap.end());
      return pos->second;
    }

    /** Removes the object with the given id from the map, if there exists one. */
    void remove(key_type id) {
      mMap.erase(id);
    }

    /** Puts the given object into this map, assigning a unique identifier to it.
     * 
     * @param value object to put into this map.
     * @returns a unique identifier assigned to the added object. */
    int put(const mapped_type& value) {
      mMap.insert(std::make_pair(mFirstFreeId, value));
      mFirstFreeId++;
      return mFirstFreeId - 1;
    }

    bool contains(key_type id) const {
      return mMap.find(id) != mMap.end();
    }

    iterator begin() {
      return mMap.begin();
    }

    const_iterator begin() const {
      return mMap.begin();
    }

    iterator end() {
      return mMap.end();
    }

    const_iterator end() const {
      return mMap.end();
    }

  private:
    map_type mMap;
    int mFirstFreeId;
  };


} // namespace smart

#endif // __SMART_IDMAP_V2_H__
