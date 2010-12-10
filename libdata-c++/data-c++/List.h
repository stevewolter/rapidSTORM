#include <data-c++/VectorList.h>
#include <iostream>

namespace data_cpp {

template <typename T>
class List {
  public:
    struct Node {
        T content;
        Node *prev, *next;

        Node() {}
        Node(const T& t) : content(t), prev(this), next(this) {}
    };

    typedef VectorList<Node> Allocator;

    struct const_iterator {
      protected:
        const Node *i;
        friend class List<Node>;
      public:
        const_iterator() : i(NULL) {}
        const_iterator(const Node *n) : i(n) {}

        const_iterator operator++() throw() { return (i = i->next); }
        const_iterator operator++(int) throw() { return (i = i->next); }
        const_iterator operator--() throw() { return (i = i->prev); }
        const_iterator operator--(int) throw() { return (i = i->prev); }

        bool operator==(const const_iterator& o) throw() 
            { return o.i == i; }
        bool operator!=(const const_iterator& o) throw() 
            { return o.i != i; }

        operator bool() throw() { return i != NULL; }

        const T& operator*() const throw() { return i->content; }
        const T* operator->() const throw() { return &i->content; }

        operator const Node *() throw() { return i; }
    };

    struct iterator : public const_iterator {
      public:
        iterator() : const_iterator() {}
        iterator(Node *n) : const_iterator(n) {}

        iterator operator++() throw() 
            { this->i = this->i->next; return *this; }
        iterator operator++(int) throw() 
            { this->i = this->i->next; return *this; }
        iterator operator--() throw() 
            { this->i = this->i->prev; return *this; }
        iterator operator--(int i) throw() 
            { this->i = this->i->prev; return *this; }

        T& operator*() throw() 
            { return const_cast<Node*>(this->i)->content; }
        T* operator->() throw() 
            { return &const_cast<Node*>(this->i)->content; }

        operator Node *() throw() { return const_cast<Node*>(this->i); }
    };

    List(Allocator& allocator) throw()
        : allocator(allocator)
        { listHead.next = listHead.prev = &listHead; }
    List(const List& other) throw()
    : allocator(other.allocator)
    { listHead.next = listHead.prev = &listHead; }
    ~List() throw() {}

    List& operator=(const List&) throw() { 
        listHead.next = listHead.prev = &listHead; 
        return *this; 
    }

    inline iterator begin() throw() { return iterator(listHead.next); }
    inline iterator end() throw() { return iterator(&listHead); }
    inline const_iterator begin() const throw()
        { return const_iterator(listHead.next); }
    inline const_iterator end() const throw()
        { return const_iterator(&listHead); }

    inline bool empty() const throw()
        { return listHead.next == listHead.prev; }

    inline void push_back_uninitialized() throw() {
        Node* n = new(allocator.allocate()) Node();
        n->prev = listHead.prev;
        n->next = &listHead;
        listHead.prev->next = n;
        listHead.prev = n;
    }

    inline void push_back(const T& t) throw() {
        Node* n = new(allocator.allocate()) Node(t);
        n->prev = listHead.prev;
        n->next = &listHead;
        listHead.prev->next = n;
        listHead.prev = n;
    }

    inline static void splice(iterator pos, iterator node) throw()
        { splice(pos, node, ((Node*)node)->next); }
    inline static void splice(iterator pos, iterator start, iterator end) 
        throw()
    {
        if (start == end) return;
        Node *p = pos, *first = start, *last = ((Node*)end)->prev;
        /* Release from old list */
        first->prev->next = last->next;
        last->next->prev = first->prev;
        /* Insert into new list */
        first->prev = p->prev;
        last->next = p;
        p->prev->next = first;
        p->prev = last;
    }

    static std::ostream &print(Node *n, std::ostream &o) throw() {
        Node *showing = n;
        do {
            o << " " << showing << " " << showing->next << " " <<
                 showing->prev << "\n";
            showing = showing->next;
        } while (showing != n);
        return o;
    }

    std::ostream &print(std::ostream &o) throw()
        { return print(&listHead, o); }

  private:
    Node listHead;
    Allocator& allocator;
};

}
