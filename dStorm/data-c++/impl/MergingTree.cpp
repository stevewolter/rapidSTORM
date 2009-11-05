#include <dStorm/data-c++/MergingTree.h>
#include <stdlib.h>

namespace data_cpp {

template <typename ImplType>
void MergingTree<ImplType>::init() { 
    back = 0; sz = 100; 
    data = (Node*)malloc(sizeof(Node) * sz);
    limit = std::numeric_limits<int>::max();
}

template <typename ImplType>
void MergingTree<ImplType>::destroy() {
    free(data);
}

template <typename ImplType>
void MergingTree<ImplType>::doubleSpace() {
    int old_size = sz;
    sz *= 2;
    Node *new_data = (Node*)malloc(sizeof(Node) * sz);
    memcpy(new_data, data, sizeof(Node) * old_size);
    free(data);
    data = new_data;
}

template <typename ImplType>
int MergingTree<ImplType>::commit() {
    if (back != 0) {
        const ImplType &fresh = data[back].c;

        /* Vector index of current tree element in traversion. */
        int n = 0;
        /* Number of elements to the left of the current. */
        int toLeft = 0;
        while (true) {
            Node &cur = data[n];
            int comparison = 
#ifdef DATA_CPP_MERGINGTREE_INLINE_COMPARISON
                (fresh < cur.c) ? -1 : 1;
#else
                compare(fresh, cur.c);
#endif

            /* Merge equal elements. */
            if (comparison == 0) {
                merge(cur.c, fresh);
                return -2;
            }

            /* Select left or right subtree based on comparison result. */
            int &next = (comparison < 0) ? cur.l : cur.r;
            if ( comparison > 0 ) {
                toLeft += cur.nl + 1 /* 1 for the node itself */;
                if (toLeft > limit) { cur.nr++; return -1; }
            }
            /* Reached leaf? Then insert element. */
            if (next == -1) { 
                /* Link to the current element. */
                next = back;
                Node &insert = data[back];
                insert.t = n;
                insert.l = insert.r = -1;
                insert.nl = insert.nr = 0;

                /* Update subtree counts. */
                int cat = n, last = back;
                while (cat != -1) {
                    Node &node = data[cat];
                    ((last == node.l) ? node.nl : node.nr)++;
                    last = cat;
                    cat = node.t;
                }

                back++;
                return back-1;
            } else {
                n = next;
            }
        }
    } else {
        Node &insert = data[0];
        insert.t = -1;
        insert.l = insert.r = -1;
        insert.nl = insert.nr = 0;
        back = 1;
        return 0;
    }
}

template <typename ImplType>
MergingTree<ImplType>::const_iterator::const_iterator(const MergingTree &tree)
{
    rest = tree.limit;
    if (tree.back) {
        base = tree.data; 
        current = &tree.data[0];
        while (current->l != -1) 
            current = &base[current->l];
        phase = Printed;
    } else {
        current = NULL; 
        phase = AscendedRight;
    }
}

template <typename ImplType>
void MergingTree<ImplType>::const_iterator::operator++() {
    /* In the phase variable, the iterator remembers the last action.
     * The tree is scanned in pre-order; each subtree is first
     * descended through the left branches to the leftmost element 
     * in Descending state,
     * and then ascended backwards in AscendedLeft state. Every
     * time an element is reached through its left subtree, the
     * iterator pauses and then descends into the right subtree.
     * When the right subtree has been finished, it backtracks
     * the tree, checking for right subtrees that were not
     * traversed yet. */
    while (true) {
        switch (phase) {
          case Descended:
            if (current->l != -1)
                current = &base[current->l];
            else
                phase = AscendedLeft;
            break;
          case AscendedLeft:
            phase = Printed;
            rest--;
            return;
          case Printed:
            if (current->r != -1) {
                current = &base[current->r];
                phase = Descended;
            } else
                phase = AscendedRight;
            break;
          case AscendedRight:
            if (current->t == -1) { current = NULL; return; }
            int last = current - base;
            current = &base[current->t];
            if (last == current->l)
                phase = AscendedLeft;
            else
                phase = AscendedRight;
            break;
        }
    }
}

}
