#ifndef DSTORM_TRAITS_SCALAR_ITERATOR_H
#define DSTORM_TRAITS_SCALAR_ITERATOR_H

#include "scalar.h"
#include <iterator>

namespace dStorm {
namespace traits {

template <typename TraitsType>
struct Scalar<TraitsType, true>::Iterator
: public boost::iterator_facade< Iterator, Scalar<TraitsType, true>, std::random_access_iterator_tag >
{
  private:
    int field;
    mutable Scalar obj;
    Iterator(int field) : field(field) {}

    friend class boost::iterator_core_access;
    friend class Scalar<TraitsType, true>;

    Scalar& dereference() const { return obj; }
    bool equal(const Iterator& o) const { return field == o.field; }
    void increment() { ++field; }
    void decrement() { --field; }
    void advance(int n) { field += n; }
    int distance_to(const Iterator& o) const { return field - o.field; }
};

template <typename TraitsType>
typename Scalar<TraitsType, true>::Iterator Scalar<TraitsType, true>::begin()
{ return Iterator(0); }
template <typename TraitsType>
typename Scalar<TraitsType, true>::Iterator Scalar<TraitsType, true>::end()
{ return Iterator(1); }

template <typename TraitsType>
struct Scalar<TraitsType, false>::Iterator
: public boost::iterator_facade< Iterator, Scalar<TraitsType, false>, std::random_access_iterator_tag >
{
  private:
    int row, col;
    const int rows;
    mutable Scalar obj;
    Iterator(int row, int col, int rows) : row(row), col(col), rows(rows), obj(row, col) {}

    friend class boost::iterator_core_access;
    friend class Scalar<TraitsType, false>;

    Scalar& dereference() const { return (obj = Scalar(row, col)); }
    bool equal(const Iterator& o) const { return row == o.row && col == o.col; }
    void increment() { if ( ++row >= rows ) { row = 0; ++col; } }
    void decrement() { if ( --row < 0 ) { row = rows-1; --col; } }
    void advance(int n) { row += n; while ( row < 0 ) { row += rows; --col; } while ( row >= rows ) { row -= rows; ++col; } }
    int distance_to(const Iterator& o) const { return rows * (col - o.col) + (row - o.rows); }
};

template <typename TraitsType>
typename Scalar<TraitsType, false>::Iterator Scalar<TraitsType, false>::begin()
{ return Iterator(0, 0, TraitsType::Rows); }
template <typename TraitsType>
typename Scalar<TraitsType, false>::Iterator Scalar<TraitsType, false>::end()
{ return Iterator(0, TraitsType::Cols, TraitsType::Rows); }

}
}

#endif
