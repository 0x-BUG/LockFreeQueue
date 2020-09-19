/* BSD 2-Clause License



Copyright (c) 2020, sudden-death-bug

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

#ifndef NODE_H
#define NODE_H

#include <memory>
#include <atomic>

namespace node_type
{

    template<typename T>
    struct Node
    {
        T data_;
        Node* next_;

        Node() : next_(nullptr) {}
        explicit Node(const T& data)
            : data_(data),
            next_(nullptr)
        {
        }
        explicit Node(T&& data)
            : data_(std::move(data)),
            next_(nullptr)
        {
        }
        ~Node() {}
    };

    template<typename T>
    struct NodeWithHazardPointer
    {
        T data_;
        std::atomic<NodeWithHazardPointer*> next_;
        std::atomic<NodeWithHazardPointer*> hpNext_;

        NodeWithHazardPointer() : next_(nullptr), hpNext_(nullptr) {}
        explicit NodeWithHazardPointer(const T& data)
            : data_(data),
            next_(nullptr),
            hpNext_(nullptr)
        {
        }
        explicit NodeWithHazardPointer(T&& data)
            : data_(std::move(data)),
            next_(nullptr),
            hpNext_(nullptr)
        {
        }
        ~NodeWithHazardPointer() {}
    };

}

template <typename Node>
std::atomic<Node*>& GetNextNode(Node* node)
{
    return node->next_;
}

template<typename Node>
std::atomic<Node*>& GetHpNextNode(Node* node)
{
    return node->hpNext_;
}

#endif
