/* Copyright (c) 2020, yoo.huang_@outlook.com

All rights reserved.



Redistribution and use in source and binary forms, with or without

modification, are permitted provided that the following conditions are met:



1. Redistributions of source code must retain the above copyright notice, this

   list of conditions and the following disclaimer.



2. Redistributions in binary form must reproduce the above copyright notice,

   this list of conditions and the following disclaimer in the documentation

   and/or other materials provided with the distribution.



THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"

AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE

IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE

DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE

FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL

DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR

SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER

CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,

OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE

OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CHAIN_H
#define CHAIN_H

#include <memory>
#include <type_traits>

template<typename Node, typename Func>
class Chain
{
private:
    Node* head_;
    Node* tail_;
    const Func getNextPointer_;

public:
    using NodeType = Node;

    const Chain& operator=(const Chain&) = delete;
    explicit Chain(const Chain&) = delete;
    explicit Chain(const Func& getNextPointer)
        : head_(nullptr),
        tail_(nullptr),
        getNextPointer_(getNextPointer)
    {
    }
    ~Chain()
    {
    }

    constexpr NodeType* head() const
    {
        return head_;
    }
    constexpr NodeType* tail() const
    {
        return tail_;
    }
    NodeType* moveHead()
    {
        const auto tmp(head());
        head_ = nullptr;
        return tmp;
    }
    NodeType* moveTail()
    {
        const auto tmp(tail());
        tail_ = nullptr;
        return tmp;
    }
    void pushBack(NodeType* node)
    {
        if (!head_ && tail_)
        {
            fprintf(stderr, "Chain::pushBack() !head_ && tail_\n");
            return;
        }
        if (!head_)
        {
            head_ = node;
        }
        if (!tail_)
        {
            tail_ = node;
        }
        if (tail_ == node)
        {
            return;
        }
        getNextPointer_(tail_).store(node,
            std::memory_order_relaxed);
        tail_ = getNextPointer_(tail_).load(
            std::memory_order_relaxed);
    }
    void pushFront(NodeType* node)
    {
        getNextPointer_(node).store(head_,
            std::memory_order_relaxed);
        head_ = node;
        if (!tail_)
        {
            tail_ = node;
        }
    }
    constexpr bool isEmpty() const
    {
        return !head_;
    }
};

template<typename Node>
class Chain<Node, void>
{
private:
    Node* head_;
    Node* tail_;

public:
    using NodeType = Node;

    const Chain& operator=(const Chain&) = delete;
    explicit Chain(const Chain&) = delete;
    explicit Chain()
        : head_(nullptr),
        tail_(nullptr)
    {
    }
    ~Chain()
    {
    }

    constexpr NodeType* head() const
    {
        return head_;
    }
    constexpr NodeType* tail() const
    {
        return tail_;
    }
    NodeType* moveHead()
    {
        const auto tmp(head());
        head_ = nullptr;
        return tmp;
    }
    NodeType* moveTail()
    {
        const auto tmp(tail());
        tail_ = nullptr;
        return tmp;
    }
    void pushBack(NodeType* node)
    {
        if (!head_ && tail_)
        {
            fprintf(stderr, "Chain::pushBack() !head_ && tail_");
            return;
        }
        if (!head_)
        {
            head_ = node;
        }
        if (!tail_)
        {
            tail_ = node;
        }
        if (tail_ == node)
        {
            return;
        }
        tail_->next_.store(node, std::memory_order_relaxed);
        tail_ = tail_->next_.load(std::memory_order_relaxed);
    }
    void pushFront(NodeType* node)
    {
        node->next_.store(head_, std::memory_order_relaxed);
        head_ = node;
        if (!tail_)
        {
            tail_ = node;
        }
    }
    constexpr bool isEmpty() const
    {
        return !head_;
    }
};

#include "Node.h"

template<typename T>
class Chain<node_type::Node<T>, void>
{
private:
    using Node = node_type::Node<T>;
    Node* head_;
    Node* tail_;

public:
    using NodeType = Node;

    const Chain& operator=(const Chain&) = delete;
    explicit Chain(const Chain&) = delete;
    explicit Chain()
        : head_(nullptr),
        tail_(nullptr)
    {
    }
    ~Chain()
    {
    }

    constexpr NodeType* head() const
    {
        return head_;
    }
    constexpr NodeType* tail() const
    {
        return tail_;
    }
    NodeType* moveHead()
    {
        const auto tmp(head());
        head_ = nullptr;
        return tmp;
    }
    NodeType* moveTail()
    {
        const auto tmp(tail());
        tail_ = nullptr;
        return tmp;
    }
    void pushBack(NodeType* node)
    {
        if (!head_ && tail_)
        {
            fprintf(stderr, "Chain::pushBack() !head_ && tail_\n");
            return;
        }
        if (!head_)
        {
            head_ = node;
        }
        if (!tail_)
        {
            tail_ = node;
        }
        if (tail_ == node)
        {
            return;
        }
        tail_->next_;
        tail_ = tail_->next_;
    }
    void pushFront(NodeType* node)
    {
        node->next_ = head_;
        head_ = node;
        if (!tail_)
        {
            tail_ = node;
        }
    }
    constexpr bool isEmpty() const
    {
        return !head_;
    }
};

#endif
