#ifndef DSTORM_GUF_FITPOSITION_HPP
#define DSTORM_GUF_FITPOSITION_HPP

#include "guf/MultiKernelModel.h"
#include <vector>
#include <boost/bind/bind.hpp>

namespace dStorm {
namespace guf {

struct add_kernel {
    typedef void result_type;
    void operator()( std::vector<gaussian_psf::BaseExpression*>& v, 
                     gaussian_psf::BaseExpression& model ) const 
        { v.push_back( &model ); }
    void operator()( std::vector<gaussian_psf::BaseExpression*>&, 
                     constant_background::Expression& ) const {}
};

template <typename Expression>
MultiKernelModel::MultiKernelModel( Expression& e ) 
: constant( &e.get_part( boost::mpl::int_<1>() ) )
{
    for_each( e.parts, boost::bind( add_kernel(), boost::ref(gauss), _1 ) );
    
}

}
}

#endif
