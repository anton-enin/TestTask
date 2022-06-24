#pragma once
#include <queue>
#include <cassert>
#include <memory>
#include <mutex>
#include <optional>
#include <condition_variable>

template<typename T>
class MessageQueue
{
public:
    MessageQueue(size_t maxSize):
        m_head(new Node), m_tail(m_head.get()), m_maxSize(maxSize), m_currentSize(0)
    {
        assert(m_maxSize != m_currentSize);
    }
    MessageQueue(const MessageQueue& other)=delete;
    MessageQueue& operator=(const MessageQueue& other)=delete;

    //pop has to be blocking
    std::shared_ptr<T> pop()
    {
        std::unique_ptr<Node> const oldHead=waitPopHead();
        return oldHead->data;
    }

    //push has to be non-blocking
    bool push(T newValue)
    {
        std::shared_ptr<T> newData(std::make_shared<T>(std::move(newValue)));
        std::unique_ptr<Node> p(new Node);
        Node* const newTail = p.get();

        std::unique_lock<std::mutex> tailLock(m_tailMutex, std::defer_lock);
        if(!tailLock.try_lock())
            return false;
        if(m_currentSize == m_maxSize)
            return false;

        m_tail->data=newData;
        m_tail->next=std::move(p);
        m_tail = newTail;
        ++m_currentSize;

        return true;
    }

private:
    struct Node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<Node> next;
    };

    std::mutex m_headMutex;
    std::unique_ptr<Node> m_head;
    std::mutex m_tailMutex;
    Node* m_tail;
    size_t m_maxSize;
    size_t m_currentSize;
    std::condition_variable m_dataCond;

    Node* getTail()
    {
        std::lock_guard<std::mutex> tailLock(m_tailMutex);
        return m_tail;
    }

    std::unique_lock<std::mutex> waitForData()
    {
        std::unique_lock<std::mutex> headLock(m_headMutex);
        m_dataCond.wait(headLock,[&]{ return m_head.get() != getTail(); });
        return std::move(headLock);
    }

    std::unique_ptr<Node> waitPopHead()
    {
        std::unique_lock<std::mutex> headLock(waitForData());
        return popHead();
    }

    std::unique_ptr<Node> popHead()
    {
        std::unique_ptr<Node> oldHead=std::move(m_head);
        m_head=std::move(oldHead->next);
        std::lock_guard<std::mutex> tailMutex(m_tailMutex);
        --m_currentSize;
        return oldHead;
    }
};
