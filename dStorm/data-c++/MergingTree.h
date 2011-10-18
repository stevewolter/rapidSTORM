#ifndef DSTORM_MERGING_TREE_H
#define DSTORM_MERGING_TREE_H

#include <string.h>
#include <iostream>
#include <limits>
#include <boost/utility.hpp>

namespace data_cpp {

/** The MergingTree is a sorted, binary tree with two special
 *  properties: Firstly, it merges equal elements according to
 *  a merge() function into a single node; secondly, it is
 *  supplied with a limit, which gives the number of nodes that
 *  will be considered significant and will be saved. At any time,
 *  only the /limit/ smallest nodes will be saved in the tree.
 **/
template<typename T> class MergingTree 
: boost::noncopyable
{
  private:
    struct Node { int l, r, nl, nr, t; T c; };

    Node *data;
    int back, sz, limit;

    void init();
    void destroy();
    void doubleSpace();
    void ensureSpace() { if (back >= sz) doubleSpace(); }

 protected:
    /** Comparison function.
     *  \return -1 if a is smaller than b, 0 if equal, 1 if greater.
     **/
    virtual int compare(const T& a, const T &b) = 0;
    /** Merge b into a. */
    virtual void merge(T& a, const T &b) = 0;

 public:
    /** Construct empty MergingTree. */
    MergingTree() { init(); }
    /** Destructor. */
    virtual ~MergingTree() { destroy(); }

    /** Set the node limit to this number. */
    void setLimit(int limit) { this->limit = limit; }
    /** Sort the tree. It is automatically sorted; this function
     *  is here for compatibility. */
    inline void sort() const {}
    /** Returns a reference to the first element. */
    inline const T& front() { return *begin(); }
    /** Returns true if the tree is empty. */
    inline bool empty() { return back == 0; }
    /** Removes all elements from tree. */
    inline void clear() { back = 0; }

    /** Get storage space for a new element. This element is not
     *  yet added into the tree, but will if commit() is called. */
    T *add() {
        ensureSpace();
        return &data[back].c;
    }

    /** Insert the last add()ed element into the tree.
     *  @return The index of the element, or -1 if it was discarded
     *          or -2 if it was merged into another object. */
    int commit();

    /** Get a reference to the element inserted with index \c n. */
    T &get(const int n) { return data[n].c; }
    const T &get(const int n) const { return data[n].c; }

    class const_iterator;
    typedef const_iterator iterator;
    iterator begin() { return iterator(*this); }
    iterator end() { return iterator(); }
    iterator begin() const { return const_iterator(*this); }
    iterator end() const { return const_iterator(); }
    unsigned int size() const { return std::min<int>(back,limit); }

    /** Standard STL-compatible iterator. This class has only a const
     *  iterator since changing elements in a sorted tree would be
     *  very inefficient. */
    class const_iterator {
        protected:
            enum Phase { Descended, AscendedLeft, AscendedRight, Printed };
            const Node* current; Phase phase;
            const Node* base;
            int rest;

            friend class MergingTree;
            const_iterator(const MergingTree &tree);
            const_iterator()
                { current = NULL; rest = 0; phase = AscendedRight; }

        public:
            inline const T& operator*() const { return current->c; }
            inline const T* operator->() const { return &current->c; }
            inline bool operator==(const iterator &i) const 
                { return (rest <= 0 && i.rest <= 0) || 
                        (i.current == current && i.phase == phase); }
            inline bool operator!=(const iterator &i) const 
                { return (rest > 0 || i.rest > 0) && 
                        (i.current != current || i.phase != phase); }

            inline void operator++(int) { ++(*this); }
            void operator++();

            /** Would this iterator return more elements? */
            inline bool hasMore() const 
                { return rest > 0 && current != NULL; }
            /** Has this iterator reached the limit of the tree, but
             *  more elements would be available with a higher limit?
             */
            inline bool limitReached() const
                { return rest <= 0; }
            /** Has this iterator seen all elements, regardless of
             *  the limit? */
            inline bool seenAll() const
                { return current == NULL; }
    };

};

}

#endif
