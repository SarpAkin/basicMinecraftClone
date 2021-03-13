#pragma once

#include <vector>
#include <deque>
#include <mutex>

template<typename T>
class T_queue
{
private:
    std::deque<T> deq;
    std::mutex mut;
public:
    T_queue() = default;

    T pop_front()
    {
        mut.lock();
        if (deq.size() == 0)
        {
            mut.unlock();
            throw "queue is empty!";
        }
        T temp = std::move(deq.front());
        deq.pop_front();
        mut.unlock();
        return temp;
    }

    void push_back(T temp)
    {
        mut.lock();
        deq.push_back(std::move(temp));
        mut.unlock();
    }

    void push_back(std::vector<T> temp)
    {
        mut.lock();
        for(auto& t : temp)
        {
            deq.push_back(std::move(t));
        }
        mut.unlock();
    }

    std::deque<T> GetDeque()
    {
        mut.lock();
        auto temp = std::move(deq);
        mut.unlock();
        return temp;
    }

    inline size_t size()
    {
        return deq.size();
    }
};
