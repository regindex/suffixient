#include <iostream>
#include <queue>
#include <vector>
#include <functional>

// template pririty queue class
template <typename T>
class PriorityQueue {
public:

    PriorityQueue()
        : pq(comp) {}

    // push elemeny in the priority queue
    void push(const T& item)
    {
        pq.push(item);
    }

    // remove the top element
    void pop()
    {
        if (!pq.empty())
            pq.pop();
    }

    // pop element from the priority queue (without removing the element)
    T top() const {
        if (!pq.empty()) {
            return pq.top();
        }
        throw std::runtime_error("Priority queue is empty");
    }

    // return yes if the priority queue is empty false otherwise
    bool empty() const
    {
        return pq.empty();
    }

    // return the size of the priority queue
    size_t size() const
    {
        return pq.size();
    }

private:
    // Lambda function for ordering elements in the priority queue
    std::function<bool(const T&, const T&)> comp = [](const T& a, const T& b){ return *a.first > *b.first; };
    std::priority_queue<T, std::vector<T>, std::function<bool(const T&, const T&)>> pq;
};