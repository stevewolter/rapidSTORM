#include "ScheduleIndexFinder.h"
#include <nonlinfit/plane/fwd.h>
#include <nonlinfit/index_of.h>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>

namespace dStorm {
namespace fit_window {

template <typename S>
struct ScheduleIndexFinder::create_table
{
    const bool do_disjoint, use_doubles;
    typedef void result_type;

    create_table( bool allow_disjoint_fitting, bool use_doubles ) 
        : do_disjoint( allow_disjoint_fitting ), use_doubles(use_doubles) {}

    template <int ChunkSize, typename Num, typename P1, typename P2>
    bool is_appropriate( 
        nonlinfit::plane::Disjoint<Num,ChunkSize,P1,P2> t,
        int width
    ) const { 
        const int slack = boost::is_same<Num,float>::value ? 1 : 0;
        return do_disjoint && 
            (ChunkSize >= (width-slack))  &&
            (ChunkSize < (width + 2 + slack)) &&
            (boost::is_same<Num,float>::value || use_doubles);
    }

    template <int ChunkSize, typename Num, typename P1, typename P2>
    bool is_appropriate( 
        nonlinfit::plane::Joint<Num,ChunkSize,P1,P2>, int
    ) const { 
        return (boost::is_same<Num,float>::value || use_doubles); 
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

template <typename Schedule>
ScheduleIndexFinder::ScheduleIndexFinder( Schedule, bool disjoint, bool use_doubles, const Optics& optics, int max_width )
: optics(optics)
{
    bool do_disjoint = disjoint && optics.supports_guaranteed_row_width();
    for (int window_width = 0; window_width < max_width; ++window_width ) {
        table.push_back( boost::mpl::size<Schedule>::value - 1 );
        boost::mpl::for_each< Schedule >( 
            boost::bind( create_table<Schedule>(do_disjoint,use_doubles), 
                         boost::ref(table.back()), window_width, boost::placeholders::_1 ) );
    }
}


}
}
