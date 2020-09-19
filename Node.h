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
