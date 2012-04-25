#include "ScheduleIndexFinder.h"
#include <nonlinfit/plane/fwd.h>
#include <nonlinfit/index_of.h>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>

namespace dStorm {
namespace guf {

template <typename S>
struct ScheduleIndexFinder::set_if_appropriate
{
    typedef void result_type;
    const ScheduleIndexFinder& m;

    set_if_appropriate( const ScheduleIndexFinder& m ) : m(m) {}

    template <int ChunkSize, typename Num, typename P1, typename P2>
    bool is_appropriate( 
        nonlinfit::plane::Disjoint<Num,ChunkSize,P1,P2> t,
        int width
    ) const { 
        const int slack = boost::is_same<Num,float>::value ? 1 : 0;
        return m.do_disjoint && 
            (ChunkSize >= (width-slack))  &&
            (ChunkSize < (width + 2 + slack)) &&
            (boost::is_same<Num,float>::value || m.use_doubles);
    }

    template <int ChunkSize, typename Num, typename P1, typename P2>
    bool is_appropriate( 
        nonlinfit::plane::Joint<Num,ChunkSize,P1,P2>, int
    ) const { 
        return (boost::is_same<Num,float>::value || m.use_doubles); 
    }

    template <typename Tag>
    void operator()( int& result, int window_width, Tag t ) const
    {
        if ( is_appropriate( t, window_width ) ) {
            const int index = nonlinfit::index_of<S,Tag>::value;
            result = std::min( result, index );
        }
    }
};

template <typename S>
int ScheduleIndexFinder::get_evaluation_tag_index( S, const guf::Spot& position ) const
{
    int rv = boost::mpl::size<S>::value;
    boost::mpl::for_each< S >( 
        boost::bind( 
            set_if_appropriate<S>(*this), boost::ref(rv), optics.get_fit_window_width(position), _1 ) );
    if ( rv == boost::mpl::size<S>::value )
        throw std::logic_error("No appropriate-sized fitter found");
    return rv;
}

}
}
