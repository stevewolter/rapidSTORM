#include <dStorm/data-c++/VectorList.h>
#include <iostream>

namespace data_cpp {

template <typename T, unsigned int Alignment = 0x1>
class List {
  public:
    struct Node {
        T content;
        Node *prev, *next;

        Node() {}
        Node(const T& t) : content(t) {}
    };

    typedef VectorList<Node, Alignment> Allocator;
    typedef T value_type;

    struct const_iterator {
      protected:
        const Node *i;
        friend class List<Node>;
      public:
        const_iterator() : i(NULL) {}
        const_iterator(const Node *n) : i(n) {}

        const_iterator operator++() { return (i = i->next); }
        const_iterator operator++(int) { return (i = i->next); }
        const_iterator operator--() { return (i = i->prev); }
        const_iterator operator--(int) { return (i = i->prev); }

        bool operator==(const const_iterator& o) 
            { return o.i == i; }
        bool operator!=(const const_iterator& o) 
            { return o.i != i; }

        operator bool() { return i != NULL; }

        const T& operator*() const { return i->content; }
        const T* operator->() const { return &i->content; }

        operator const Node *() { return i; }
    };

    struct iterator : public const_iterator {
      public:
        iterator() : const_iterator() {}
        iterator(Node *n) : const_iterator(n) {}

        iterator operator++() 
            { this->i = this->i->next; return *this; }
        iterator operator++(int) 
            { this->i = this->i->next; return *this; }
        iterator operator--() 
            { this->i = this->i->prev; return *this; }
        iterator operator--(int i) 
            { this->i = this->i->prev; return *this; }

        T& operator*() 
            { return const_cast<Node*>(this->i)->content; }
        T* operator->() 
            { return &const_cast<Node*>(this->i)->content; }

        operator Node *() { return const_cast<Node*>(this->i); }
    };

    List(Allocator& allocator)
        : allocator(allocator)
        { listHead.next = listHead.prev = &listHead; }
    List(const List& other)
    : allocator(other.allocator)
    { listHead.next = listHead.prev = &listHead; }
    ~List() {}

    List& operator=(const List&) { 
        listHead.next = listHead.prev = &listHead; 
        return *this; 
    }

    inline iterator begin() { return iterator(listHead.next); }
    inline iterator end() { return iterator(&listHead); }
    inline const_iterator begin() const
        { return const_iterator(listHead.next); }
    inline const_iterator end() const
        { return const_iterator(&listHead); }

    inline bool empty() const { 
        return listHead.next == &listHead; 
    }
    int size() const {
        int size = 0;
        const Node *cat = &listHead;
        while ( (cat = cat->next) != &listHead )
            size++;
        return size;
    }

    inline void push_back(const T& t) {
        Node* n = new(allocator.allocate()) Node(t);
        n->prev = listHead.prev;
        n->next = &listHead;
        listHead.prev->next = n;
        listHead.prev = n;
    }

    inline void push_back() {
        Node* n = new(allocator.allocate()) Node();
        n->prev = listHead.prev;
        n->next = &listHead;
        listHead.prev->next = n;
        listHead.prev = n;
    }

    inline static void splice(iterator pos, iterator node)
        { splice(pos, node, ((Node*)node)->next); }
    inline static void splice(iterator pos, iterator start, iterator end) 

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

    static std::ostream &print(const Node *n, std::ostream &o) {
        const Node *showing = n;
        do {
            o << " " << showing << " " << showing->next << " " <<
                 showing->prev << "\n";
            showing = showing->next;
        } while (showing != n);
        return o;
    }

    std::ostream &print(std::ostream &o)
        { return print(&listHead, o); }

  private:
    Node listHead;
    Allocator& allocator;
};

}
