#ifndef _TIT_MAP_H_
#define _TIT_MAP_H_
#include <string>
#include <map>
#include <mutex>

template <typename T>
class TIT_Map{
public:
    TIT_Map(){}
    ~TIT_Map(){}

    void insert(typename std::pair<std::string, T> in)
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        _tit_map.insert(in);
        return;
    }

    typename std::map<std::string, T>::iterator find(const std::string& sid)
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.find(sid);
    }

    typename std::map<std::string, T>::iterator erase(typename std::map<std::string, T>::iterator it)
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.erase(it);
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.empty();
    }

    typename std::map<std::string, T>::iterator begin()
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.begin();
    }

    typename std::map<std::string, T>::iterator end()
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.end();
    }

    T& operator[] (std::string& key)
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
    typename std::map<std::string, T> _tit_map;
    std::mutex                        _mutex;
};

#endif//_TIT_MAP_H_