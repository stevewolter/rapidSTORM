#ifndef DSTORM_GUF_FITPOSITION_HPP
#define DSTORM_GUF_FITPOSITION_HPP

#include "FitAnalysis.h"
#include <vector>
#include <boost/bind/bind.hpp>

namespace dStorm {
namespace guf {

struct add_plane {
    typedef void result_type;
    void operator()( std::vector<PSF::BaseExpression*>& v, 
                     PSF::BaseExpression& model ) const 
        { v.push_back( &model ); }
    void operator()( std::vector<PSF::BaseExpression*>&, 
                     constant_background::Expression& ) const {}
};

template <typename Expression>
FittedPlane::FittedPlane( Expression& e ) 
: constant( &e.get_part( boost::mpl::int_<1>() ) )
{
    for_each( e.parts, boost::bind( add_plane(), boost::ref(gauss), _1 ) );
    
}

}
}

#endif
