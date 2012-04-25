#ifndef DSTORM_FITTER_GUF_FIT_ANALYSIS_H
#define DSTORM_FITTER_GUF_FIT_ANALYSIS_H

#include "Spot.h"
#include "gaussian_psf/fwd.h"
#include "guf/constant_background_fwd.hpp"
#include <vector>
#include <boost/iterator/indirect_iterator.hpp>

namespace dStorm {
namespace guf {

struct MultiKernelModel {
    template <typename Expression>
    MultiKernelModel( Expression& expression );

    typedef boost::indirect_iterator< 
        std::vector<gaussian_psf::BaseExpression*>::const_iterator,
        const gaussian_psf::BaseExpression&> const_iterator;
    typedef boost::indirect_iterator< 
        std::vector<gaussian_psf::BaseExpression*>::iterator,
        gaussian_psf::BaseExpression&> iterator;

    const_iterator begin() const { return const_iterator(gauss.begin()); }
    const_iterator end() const { return const_iterator(gauss.end()); }
    iterator begin() { return iterator(gauss.begin()); }
    iterator end() { return iterator(gauss.end()); }
    int kernel_count() const { return gauss.size(); }

    gaussian_psf::BaseExpression& operator[]( int i ) { return *gauss[i]; }
    const gaussian_psf::BaseExpression& operator[]( int i ) const { return *gauss[i]; }
    constant_background::Expression& background_model() { return *constant; }
    const constant_background::Expression& background_model() const { return *constant; }
    
  private:
    std::vector<gaussian_psf::BaseExpression*> gauss;
    constant_background::Expression* constant;
};

struct MultiKernelModelStack : public std::vector<MultiKernelModel> 
{
};

}
}

#endif
