#ifndef NONLINFIT_PLANE_SUM_MATRIX_ROWS_H
#define NONLINFIT_PLANE_SUM_MATRIX_ROWS_H

namespace nonlinfit {
namespace plane {

template <bool Rows, bool Columns, typename Target, typename Source, typename Reducer>
static void sum_matrix_rows( Target& t, const Source& s, const Reducer& a) {
    t.fill(0);
    for (int r = 0; r < s.rows(); ++r)
        for (int c = 0; c < s.cols(); ++c)
            t( ( Rows ) ? a[r] : r, ( Columns ) ? a[c] : c ) += s(r,c);
}
/** Apply the reduction to both dimensions (rows and columns). */
template <typename Target, typename Source, typename Reducer>
void sum_rows_and_cols( Target& t, const Source& s, const Reducer& r) { sum_matrix_rows<true,true>(t,s,r); }
/** Reduce only the rows, leaving the column count constant. */
template <typename Target, typename Source, typename Reducer>
static void sum_rows( Target& t, const Source& s, const Reducer& r) { sum_matrix_rows<true,false>(t,s,r); }
/** Reduce only the columns, leaving the row count constant. */
template <typename Target, typename Source, typename Reducer>
static void sum_cols( Target& t, const Source& s, const Reducer& r) { sum_matrix_rows<false,true>(t,s,r); }

}
}

#endif
