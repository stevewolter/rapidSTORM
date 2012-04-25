#ifndef DSTORM_FIT_WINDOW_PLANECREATOR_HPP
#define DSTORM_FIT_WINDOW_PLANECREATOR_HPP

#include "PlaneCreator.h"
#include "PlaneImpl.hpp"
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>

namespace dStorm {
namespace fit_window {

template <typename Tag>
struct PlaneCreatorImpl : public PlaneCreator {
    const Optics& input;
    std::auto_ptr<Plane> extract_data_( const engine::Image2D& image, const Spot& position ) const {
        return std::auto_ptr<Plane>( new PlaneImpl<Tag>( input, image, position ) );
    }
public:
    PlaneCreatorImpl( const Optics& input ) : input(input) {}
};

template <typename EvaluationSchedule>
struct PlaneCreatorTable::instantiator {
    typedef void result_type;
    template <typename Tag>
    void operator()( PlaneCreatorTable& target, Tag ) 
    {
        target.table_.push_back( new PlaneCreatorImpl<Tag>(target.optics) );
    }
};

template <typename EvaluationSchedule>
PlaneCreatorTable::PlaneCreatorTable( 
    EvaluationSchedule, 
    const Optics& input
) : optics(input) 
{
    boost::mpl::for_each< EvaluationSchedule >(
        boost::bind( instantiator<EvaluationSchedule>(), boost::ref(*this), _1 ) );
}

}
}

#endif
