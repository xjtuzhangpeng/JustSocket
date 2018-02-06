#ifndef _TIT_MAP_H_
#define _TIT_MAP_H_
#include <string>
#include <map>
#include <mutex>

template <typename KEY, typename VALUE>
class TIT_Map{
public:
    TIT_Map(){}
    ~TIT_Map(){}

    void insert(typename std::pair<KEY, VALUE> in)
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        _tit_map.insert(in);
        return;
    }

    typename std::map<KEY, VALUE>::iterator find(const KEY& sid)
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.find(sid);
    }

    typename std::map<KEY, VALUE>::iterator erase(typename std::map<KEY, VALUE>::iterator it)
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.erase(it);
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.empty();
    }

    typename std::map<KEY, VALUE>::iterator begin()
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.begin();
    }

    typename std::map<KEY, VALUE>::iterator end()
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.end();
    }

    VALUE& operator[] (KEY& key)
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map[key];
    }

    size_t size()
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.size();
    }

private:
    typename std::map<KEY, VALUE> _tit_map;
    std::mutex                    _mutex;
};

#endif//_TIT_MAP_H_