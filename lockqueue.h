#ifndef LOCKQUEUE_H
#define LOCKQUEUE_H

#include <queue>
#include <mutex>

template <typename T>
class lockqueue {
private:
    std::queue<T> q;
    std::mutex l;
public:
    void pop() {
        l.lock();
        q.pop();
        l.unlock();
    }

    T front() {
        return q.front();
    }

    void push(T item) {
        l.lock();
        q.push(item);
        l.unlock();
    }

    bool empty() {  
        return q.empty();
    }

    decltype(auto) size() {
        return q.size();
    }

    template <class... _Args>
    decltype(auto) emplace(_Args&&... __args) {
        l.lock();
        q.emplace(_VSTD::forward<_Args>(__args)...);
        l.unlock();
    }

    void swap(std::queue<T>& x) {
        l.lock();
        q.swap(x);
        l.unlock();
    }
};

#endif