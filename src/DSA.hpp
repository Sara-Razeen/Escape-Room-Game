#ifndef DSA_HPP
#define DSA_HPP
#include <iostream>
struct Node
{
    int data;
    struct Node *left;
    struct Node *right;
};

Node *newNode(int data)
{
    Node *temp = new Node;
    temp->data = data;
    temp->left = temp->right = NULL;
    return temp;
}
template <typename T>
class stackNode
{
public:
    T data;
    stackNode *next;
    stackNode(T d) : data(d), next(NULL) {}
};


template <typename T>
class Stack
{
    stackNode<T> *top;

public:
    Stack() : top(NULL) {}

    bool IsEmpty()
    {
        return top == NULL;
    }

    void push(T d)
    {
        stackNode<T> *newstackNode = new stackNode<T>(d);
        newstackNode->next = top;
        top = newstackNode;
    }

    T Pop()
    {
        if (IsEmpty())
        {
            std::cout << "Stack is Empty" << std::endl;
            return T();
        }
        T data = top->data;
        stackNode<T> *temp = top;
        top = top->next;
        delete temp;
        return data;
    }

    T Top()
    {
        if (IsEmpty())
        {
            std::cout << "Stack is empty" << std::endl;
            return T(); 
        }
        return top->data;
    }

    int size()
    {
        int count = 0;
        stackNode<T> *ptr = top;

        while (ptr != NULL)
        {
            count++;
            ptr = ptr->next;
        }

        return count;
    }

    void display()
    {
        if (IsEmpty())
        {
            std::cout << "Stack is empty" << std::endl;
            return;
        }
        stackNode<T> *ptr = top;
        while (ptr != NULL)
        {
            std::cout << ptr->data << " ";
            ptr = ptr->next;
        }
        std::cout << std::endl;
    }
};
template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;

        Node(const T& value) : data(value), next(nullptr) {}
    };

    Node* frontNode;
    Node* rearNode;
    int queueSize;

public:
    Queue() : frontNode(nullptr), rearNode(nullptr), queueSize(0) {}

    
    void enqueue(const T& value) {
        Node* newNode = new Node(value);
        if (empty()) {
            frontNode = rearNode = newNode;
        } else {
            rearNode->next = newNode;
            rearNode = newNode;
        }
        ++queueSize;
    }

    
    void dequeue() {
        if (!empty()) {
            Node* temp = frontNode;
            frontNode = frontNode->next;
            delete temp;
            --queueSize;
        }
    }

    
    T front() const {
        if (!empty()) {
            return frontNode->data;
        }

        return T();
    }

    
    bool empty() const {
        return queueSize == 0;
    }

    int QueueSize() const {
        return queueSize;
    }


    void clear() {
        while (!empty()) {
            dequeue();
        }
    }

    
    ~Queue() {
        clear();
    }

  };

#endif 