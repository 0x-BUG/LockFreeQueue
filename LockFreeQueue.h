#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H

#include "Node.h"
#include "HazardPointer.h"
#include <atomic>
#include <memory>

template<typename T, size_t MAX_THREADS, size_t GC_NUM = 0>
class alignas(std::atomic<T*>) LockFreeQueue
    : private QueueHazardPointerIndex
{
public:
    using Node = node_type::NodeWithHazardPointer<T>;
    using Chain = Chain<Node, void>;

private:
    using Hp = QueueHazardPointerOwner<
        Node, PER_THREAD_HP_NUM, MAX_THREADS * 2>;
    MsQueue<Node> queue_;

public:
    explicit LockFreeQueue(const LockFreeQueue&) = delete;
    const LockFreeQueue& operator=(const LockFreeQueue&) = delete;

    LockFreeQueue()
        : queue_()
    {
    }

    ~LockFreeQueue()
    {
        Node* head(queue_.head_.load(std::memory_order_relaxed));
        while (head)
        {
            Node* tmp(head);
            head = head->next_.load(std::memory_order_relaxed);
            delete tmp;
        }
    }

    bool push(const T& value)
    {
        Node* newNode(new Node(value));
        std::atomic<Node*>& hazardTail(Hp::GetHazardPointer(CURRENT));
        queue_.push(hazardTail, newNode, GetNextNode<Node>);
        hazardTail.store(nullptr, std::memory_order_release);
        return true;
    }

    bool push(T&& value)
    {
        Node* newNode(new Node(std::move(value)));
        std::atomic<Node*>& hazardTail(Hp::GetHazardPointer(CURRENT));
        queue_.push(hazardTail, newNode, GetNextNode<Node>);
        hazardTail.store(nullptr, std::memory_order_release);
        return true;
    }

    bool append(Node* node)
    {
        std::atomic<Node*>& hazardTail(Hp::GetHazardPointer(CURRENT));
        bool ret(queue_.append(hazardTail, node, GetNextNode<Node>));
        hazardTail.store(nullptr, std::memory_order_release);
        return ret;
    }
    bool append(Node* first, Node* last)
    {
        std::atomic<Node*>& hazardTail(Hp::GetHazardPointer(CURRENT));
        bool ret(queue_.append(hazardTail, first, last, GetNextNode<Node>));
        hazardTail.store(nullptr, std::memory_order_release);
        return ret;
    }
    bool append(Chain& chain)
    {
        if (chain.isEmpty())
        {
            return false;
        }
        return append(chain.moveHead(), chain.moveTail());
    }

    bool pop(T& value)
    {
        std::atomic<Node*>& hazardHead(Hp::GetHazardPointer(CURRENT));
        std::atomic<Node*>& hazardNext(Hp::GetHazardPointer(NEXT));
        Node* oldHead(queue_.pop(hazardHead, hazardNext, GetNextNode<Node>));
        if (!oldHead)
        {
            hazardNext.store(nullptr);
            return false;
        }
        hazardHead.store(nullptr, std::memory_order_relaxed);
        std::swap(value, hazardNext.load()->data_);
        Hp::ReclaimLater(oldHead);
        if (Hp::Length() >= GC_NUM)
        {
            Hp::ReclaimLocalHazardNodes();
        }
        hazardNext.store(nullptr, std::memory_order_release);
        return true;
    }

    static void ReclaimLocalHazardNodes()
    {
        Hp::ReclaimLocalHazardNodes();
    }

    bool isEmpty() const
    {
        return queue_.head_.load() == queue_.tail_.load();
    }

    static void ReclaimHazardNodes()
    {
        Hp::ReclaimHazardNodes();
    }
};

#endif
