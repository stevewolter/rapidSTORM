#ifndef DSTORM_FITTER_GUF_FIT_ANALYSIS_H
#define DSTORM_FITTER_GUF_FIT_ANALYSIS_H

#include "Spot.h"
#include "gaussian_psf/SingleKernelModel.h"
#include "constant_background/fwd.hpp"
#include <vector>
#include <boost/iterator/indirect_iterator.hpp>

namespace dStorm {
namespace guf {

struct MultiKernelModel {
    typedef gaussian_psf::SingleKernelModel SingleKernelModel;
    template <typename Expression>
    MultiKernelModel( Expression& expression );

    typedef boost::indirect_iterator<
        std::vector<SingleKernelModel*>::const_iterator,
        const SingleKernelModel&> const_iterator;
    typedef boost::indirect_iterator<
        std::vector<SingleKernelModel*>::iterator,
        SingleKernelModel&> iterator;

    const_iterator begin() const { return const_iterator(gauss.begin()); }
    const_iterator end() const { return const_iterator(gauss.end()); }
    iterator begin() { return iterator(gauss.begin()); }
    iterator end() { return iterator(gauss.end()); }
    int kernel_count() const { return gauss.size(); }

    SingleKernelModel& operator[]( int i ) { return *gauss[i]; }
    const SingleKernelModel& operator[]( int i ) const { return *gauss[i]; }
    constant_background::Expression& background_model() { return *constant; }
    const constant_background::Expression& background_model() const { return *constant; }

  private:
    std::vector<gaussian_psf::SingleKernelModel*> gauss;
    constant_background::Expression* constant;
};

struct MultiKernelModelStack : public std::vector<MultiKernelModel>
{
};

}
}

#endif
