#ifndef SIMPARM_EIGEN_HH
#define SIMPARM_EIGEN_HH

#include "Eigen_decl.hh"
#include "iostream.hh"
#include <Eigen/Core>
#include "default_value.hh"
#include <stdexcept>

namespace simparm {

template <typename Derived>
std::istream& from_config_stream( std::istream& i, Eigen::MatrixBase<Derived>& t ) { 
   char sep;
   i >> std::skipws;
   for ( int r = 0; r < t.rows(); ++r ) {
        if ( r != 0 ) {
            i >> sep;
            if ( !i || sep != ',' ) 
                throw std::runtime_error("No comma found separating matrix entries");
        }
        for ( int c = 0; c < t.cols(); ++c )
            from_config_stream(i, t(r,c) );
   }
   return i;
}

template <typename Derived>
std::ostream& to_config_stream( std::ostream& o, const Eigen::MatrixBase<Derived>& t ) { 
    for (int r = 0; r < t.rows(); ++r) {
        if ( r > 0 ) o << ",";
        for (int c = 0; c < t.cols(); ++c) {
            if ( c > 0 ) o << " ";
            to_config_stream(o, t(r,c));
        }
    }
    return o;
}

template <typename Derived>
static const char *typeName( const Eigen::MatrixBase<Derived>& ) {
   return typeName( typename Derived::Scalar() );
}

}

#include "Attributes.hh"
#include "IncrementWatcher.hh"
#include "MinMaxWatcher.hh"

namespace simparm {

template <typename Scalar, int Rows, int Cols, int Flags, int MaxRows, int MaxCols, typename ValueField>
class Attributes< Eigen::Matrix<Scalar,Rows,Cols,Flags,MaxRows,MaxCols>,ValueField,void>
: public Attributes< Scalar, ValueField, void >
{
    simparm::Attribute<int> rows, columns;

  public:
    typedef Attributes< Scalar, ValueField, void > Base;
    Attributes( Attribute<ValueField>& a ) : Base(a), rows("rows", Rows), columns("columns", Cols) {}
    Attributes( const Attributes& a, Attribute<ValueField>& b ) : Base(a,b), rows(a.rows), columns(a.columns) {}
    void registerNamedEntries( simparm::NodeHandle n ) { 
        n->add_attribute(rows);
        n->add_attribute(columns);
        Base::registerNamedEntries(n);
    }
};

template <typename Derived>
inline Derived default_value( Eigen::MatrixBase<Derived> ) { 
    return Derived::Constant( default_value( typename Derived::Scalar() ) ); 
}

template <typename Derived>
inline Derived default_increment( Eigen::MatrixBase<Derived> ) { 
    return Derived::Constant( default_increment( typename Derived::Scalar() ) ); 
}

template <typename Scalar, int Rows, int Cols, int Flags, int MaxRows, int MaxCols>
class IncrementWatcher< Eigen::Matrix<Scalar,Rows,Cols,Flags,MaxRows,MaxCols> >
: public Attribute< Eigen::Matrix<Scalar,Rows,Cols,Flags,MaxRows,MaxCols> >::ChangeWatchFunction
{
    typedef Eigen::Matrix<Scalar,Rows,Cols,Flags,MaxRows,MaxCols> TypeOfEntry;

    Attribute<TypeOfEntry>& increment;
  public:
    IncrementWatcher(Attribute<TypeOfEntry>& increment) : increment(increment) {}
    bool operator()(const TypeOfEntry& a, const TypeOfEntry &b) {
        using std::abs;
        TypeOfEntry difference = (a-b);
        difference = difference.cwise().max( -difference );
        return (a.cwise() != a).any() || (b.cwise() != b).any() || 
             ( difference.cwise() >= increment()).any();
    }
};

template <typename Derived>
inline bool exceeds( const Eigen::MatrixBase<Derived>& a, const Eigen::MatrixBase<Derived>& b ) {
    return (a.array() > b.array()).any();
}

template <typename Derived>
inline bool falls_below( const Eigen::MatrixBase<Derived>& a, const Eigen::MatrixBase<Derived>& b ) {
    return (a.array() < b.array()).any();
}

}

#endif
