#ifndef DSTORM_IMAGE_NORMALIZE_H
#define DSTORM_IMAGE_NORMALIZE_H

namespace dStorm {

template <typename Input, typename Output>
struct Normalize : public std::unary_function< Input, Output > 
{
    Normalize( const std::pair< Input, Input >& range, 
               const std::pair< Output, Output >& orange ) 
        : range_(range), orange_(orange) {}

    Output operator()( const Input& i ) const 
    {
        if ( i < range_.first )
            return orange_.first;
        else if ( i > range_.second )
            return orange_.second;
        else
            return float( i - range_.first ) / float( range_.second - range_.first )
                * (orange_.second - orange_.first) + orange_.first;
    }

  private:
    std::pair< Input, Input > range_;
    std::pair< Output, Output > orange_;
};

template <typename Input, typename Output>
Normalize<Input,Output> normalize( 
    const std::pair< Input, Input >& range, 
    const std::pair< Output, Output >& orange )
    { return Normalize<Input,Output>(range, orange); }

}

#endif
