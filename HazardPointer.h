#ifndef HAZARD_POINTER_H
#define HAZARD_POINTER_H

#include "Node.h"
#include "Chain.h"
#include <thread>
#include <atomic>
#include <functional>

template<typename T>
struct alignas(std::atomic<T*>) HazardPointer
{
    std::atomic<std::thread::id> id_;
    std::atomic<T*> pointer_;
};

template<typename Node>
class ContainerOfNodes
{
public:
    using GetNextPointer = std::atomic<Node*>& (*)(Node*);
    using Chain = Chain<Node, GetNextPointer>;
};

template<typename HpNode>
void Reclaim(const HpNode* node)
{
    delete node;
}

template<typename HpNode>
class alignas(std::atomic<HpNode*>) MsQueue
{
public:
    std::atomic<HpNode*> head_;
    std::atomic<HpNode*> tail_;

    explicit MsQueue(const MsQueue&) = delete;
    const MsQueue& operator=(const MsQueue&) = delete;

    MsQueue()
        : head_(new HpNode()),
        tail_(head_.load(std::memory_order_relaxed))
    {

    }

    template<typename Handler>
    void push(std::atomic<HpNode*>& hazardPointer, HpNode* newNode,
        const Handler& getNextPointer)
    {
        HpNode* oldTail;
        for (;;)
        {
            hazardPointer.store(tail_.load(std::memory_order_relaxed));
            oldTail = hazardPointer.load(std::memory_order_acquire);
            HpNode* next(getNextPointer(oldTail).load(
                std::memory_order_acquire));
            if (next)
            {
                tail_.compare_exchange_weak(oldTail, next,
                    std::memory_order_release,
                    std::memory_order_relaxed);
                continue;
            }

            HpNode* tmp(nullptr);
            if (getNextPointer(oldTail).compare_exchange_strong(
                tmp, newNode, std::memory_order_release,
                std::memory_order_relaxed))
            {
                break;
            }
        }
        tail_.compare_exchange_strong(oldTail, newNode,
            std::memory_order_release,
            std::memory_order_relaxed);
    }

    template<typename Handler>
    HpNode* pop(std::atomic<HpNode*>& hp1, std::atomic<HpNode*>& hp2,
        const Handler& getNextPointer)
    {
        HpNode* oldHead;
        for (;;)
        {
            hp1.store(head_.load(std::memory_order_relaxed));
            oldHead = hp1.load(std::memory_order_acquire);
            if (head_.load(std::memory_order_acquire) != oldHead)
            {
                continue;
            }
            hp2.store(getNextPointer(oldHead).load(
                std::memory_order_relaxed));
            HpNode* next(hp2.load(std::memory_order_acquire));
            if (!next)
            {
                hp1.store(nullptr);
                return nullptr;
            }

            HpNode* oldTail(tail_.load(std::memory_order_acquire));
            if (oldHead == oldTail)
            {
                tail_.compare_exchange_strong(oldTail, next,
                    std::memory_order_release,
                    std::memory_order_relaxed);
                continue;
            }
            if (head_.compare_exchange_strong(
                oldHead, next,
                std::memory_order_acquire,
                std::memory_order_relaxed))
            {
                break;
            }
        }
        return oldHead;
    }

    template<typename Handler>
    bool append(std::atomic<HpNode*>& hazardPointer,
        HpNode* first, HpNode* last,
        const Handler& getNextPointer)
    {
        if (!first)
        {
            return false;
        }
        push(hazardPointer, first, getNextPointer);
        if (last && (first != last))
        {
            tail_.compare_exchange_strong(
                first, last, std::memory_order_release,
                std::memory_order_relaxed);
        }
        else if (!last)
        {
            last = first;
            auto next(getNextPointer(last).load(
                std::memory_order_relaxed));
            while (next)
            {
                last = next;
                next = getNextPointer(last).load(
                    std::memory_order_relaxed);
            }
            if (last != first)
            {
                tail_.compare_exchange_strong(last, first,
                    std::memory_order_release,
                    std::memory_order_relaxed);
            }
        }
        return true;
    }

    template<typename Handler>
    bool append(std::atomic<HpNode*>& hazardPointer,
        HpNode* node, const Handler& getNextPointer)
    {
        if (!node)
        {
            return false;
        }
        push(hazardPointer, node, getNextPointer);
        return true;
    }
};

class QueueHazardPointerIndex
{
public:
    enum { CURRENT, NEXT, PER_THREAD_HP_NUM };
};

template<typename HpNode, size_t PER_THREAD_HP_NUM, size_t LEN>
class QueueHazardPointerOwner;

template<typename HpNode, size_t LEN>
class alignas(std::atomic<HpNode*>) HazardPointersSingleton
    : private QueueHazardPointerIndex
{
private:
    using Hp = QueueHazardPointerOwner<HpNode, PER_THREAD_HP_NUM, LEN>;
    HazardPointer<HpNode> hazardPointers_[LEN];
    MsQueue<HpNode> queue_;

    HazardPointersSingleton()
        : hazardPointers_(),
        queue_()
    {
    }

public:
    ~HazardPointersSingleton()
    {
        auto head(queue_.head_.load(std::memory_order_relaxed));
        while (head)
        {
            const auto tmp(head);
            head = head->hpNext_.load(std::memory_order_relaxed);
            delete tmp;
        }
    }

    static HazardPointersSingleton<HpNode, LEN>& Instance()
    {
        static HazardPointersSingleton<HpNode, LEN> hps;
        return hps;
    }

    HazardPointer<HpNode>* get()
    {
        return hazardPointers_;
    }

    bool isExist(const HpNode* ptr)
    {
        for (const auto& iter : hazardPointers_)
        {
            if (iter.pointer_.load(std::memory_order_acquire) == ptr)
            {
                return true;
            }
        }
        return false;
    }

    bool reclaimLater(HpNode* first, HpNode* last)
    {
        std::atomic<HpNode*>& hazardTail = Hp::GetHazardPointer(CURRENT);
        return queue_.append(hazardTail, first, last, GetHpNextNode<HpNode>);
    }
    bool reclaimLater(HpNode* node)
    {
        std::atomic<HpNode*>& hazardTail = Hp::GetHazardPointer(CURRENT);
        return queue_.append(hazardTail, node, GetHpNextNode<HpNode>);
    }
    void reclaimHazardNodes()
    {
        std::atomic<HpNode*>& hazardHead(Hp::GetHazardPointer(CURRENT));
        std::atomic<HpNode*>& hazardNext(Hp::GetHazardPointer(NEXT));
        typename ContainerOfNodes<HpNode>::Chain chain(GetHpNextNode<HpNode>);
        for (;;)
        {
            HpNode* oldHead(queue_.pop(hazardHead, hazardNext,
                GetHpNextNode<HpNode>));
            if (!oldHead)
            {
                break;
            }
            hazardHead.store(nullptr);
            hazardNext.store(nullptr);
            if (!isExist(oldHead))
            {
                Reclaim(oldHead);
            }
            else
            {
                chain.pushFront(oldHead);
            }
        }
        if (chain.head())
        {
            if (!reclaimLater(chain.moveHead(), chain.moveTail()))
            {
                fprintf(stderr, "Hps::reclaimLater(HpNode*, HpNode*) failed\n");
            }
        }
    }
};

template<typename HpNode>
void InitialHazardPointer(HazardPointer<HpNode>* global,
    const size_t globalLen,
    HazardPointer<HpNode>** local,
    const size_t localLen)
{
    for (size_t i = 0; i < localLen; ++i)
    {
        for (size_t j = 0; j < globalLen; ++j)
        {
            std::thread::id old;
            if (global[j].id_.compare_exchange_strong(
                old, std::this_thread::get_id(),
                std::memory_order_seq_cst,
                std::memory_order_relaxed))
            {
                local[i] = &global[j];
                break;
            }
        }
    }
}

template<typename HpNode, size_t PER_THREAD_HP_NUM, size_t LEN>
class alignas(std::atomic<HpNode*>) QueueHazardPointerOwner
{
private:
    using Hps = HazardPointersSingleton<HpNode, LEN>;

    HazardPointer<HpNode>* hp_[PER_THREAD_HP_NUM];
    typename ContainerOfNodes<HpNode>::Chain chain_;
    size_t count_;

    QueueHazardPointerOwner()
        : hp_{ nullptr },
        chain_(GetHpNextNode<HpNode>),
        count_(0)
    {
        HazardPointer<HpNode>* hps(Hps::Instance().get());
        InitialHazardPointer(hps, LEN, hp_, PER_THREAD_HP_NUM);
        if (!hp_[PER_THREAD_HP_NUM - 1])
        {
            fprintf(stderr, "get hazard pointer failed\n");
        }
    }

    static QueueHazardPointerOwner<
        HpNode, PER_THREAD_HP_NUM, LEN>& Instance()
    {
        static thread_local QueueHazardPointerOwner<
            HpNode, PER_THREAD_HP_NUM, LEN> hp;
        return hp;
    }

    std::atomic<HpNode*>& get(size_t index)
    {
        return hp_[index]->pointer_;
    }

public:
    explicit QueueHazardPointerOwner(
        const QueueHazardPointerOwner&) = delete;
    const QueueHazardPointerOwner& operator=(
        const QueueHazardPointerOwner&) = delete;

    ~QueueHazardPointerOwner()
    {
        ReclaimLocalHazardNodes();
        if (count_)
        {
            bool ret(true);
            if (count_ == 1)
            {
                ret = Hps::Instance().reclaimLater(chain_.head());
            }
            else
            {
                ret = Hps::Instance().reclaimLater(
                    chain_.head(), chain_.tail());
            }
            if (!ret)
            {
                fprintf(stderr, "Hps::reclaimLater(HpNode*, HpNode*) failed\n");
            }
        }
        for (const auto& iter : hp_)
        {
            iter->pointer_.store(nullptr);
            iter->id_.store(std::thread::id());
        }
    }

    static std::atomic<HpNode*>& GetHazardPointer(size_t index)
    {
        return Instance().get(index);
    }

    static void ReclaimLater(HpNode* hazard)
    {
        Instance().chain_.pushFront(hazard);
        ++Instance().count_;
    }

    static void ReclaimLocalHazardNodes()
    {
        HpNode* current(Instance().chain_.moveHead());
        Instance().chain_.moveTail();
        while (current)
        {
            HpNode* const next(current->hpNext_.load(
                std::memory_order_relaxed));
            if (!Hps::Instance().isExist(current))
            {
                Reclaim(current);
            }
            else
            {
                ReclaimLater(current);
            }
            --Instance().count_;
            current = next;
        }
    }

    static void ReclaimHazardNodes()
    {
        ReclaimLocalHazardNodes();
        Hps::Instance().reclaimHazardNodes();
    }

    constexpr static size_t Length()
    {
        return Instance().count_;
    }
};

#endif
