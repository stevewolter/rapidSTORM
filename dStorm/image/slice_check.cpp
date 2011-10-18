#include "../helpers/thread.h"
#include "slice.h"
#include "constructors.h"
#include "iterator.h"

using namespace dStorm;
using namespace boost::units;

int main() {
    ost::DebugStream::set(std::cerr);
    Image<int,4> a( Image<int,4>::Size::Constant(25*camera::pixel) );
    for ( Image<int,4>::iterator i = a.begin(); i != a.end(); ++i ) {
        *i = 0;
        for (int j = 0; j < 4; ++j) *i = *i * 100 + i.position()[j];
    }

    Image<int,2> b( a.slice(2,13*camera::pixel).slice(1,24*camera::pixel) );
    int count = 0;
    for ( Image<int,2>::iterator i = b.begin(); i != b.end(); ++i ) {
        int s = 0;
        for (int j = 0; j < 4; ++j) 
            s = s * 100 + ( (j == 1) ? 24 : (j == 2) ? 13 : (j == 3) ? i.position()[1] : i.position()[0] );
        assert( s == *i );
        ++count;
    }
    assert( count == 25*25 );
    return 0;
}
