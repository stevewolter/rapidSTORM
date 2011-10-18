#ifndef DSTORM_TRAITS_RESOLUTION_H
#define DSTORM_TRAITS_RESOLUTION_H

namespace dStorm {
namespace traits {

template <typename Base, typename Scalar>
class Resolution {
  public:
    static const bool has_resolution = true;
    typedef typename value<Base>::MoS::template value<boost::optional<Scalar> >::type
        ResolutionType;
    typedef Scalar user_resolution_type;

  private:
    ResolutionType _resolution;
  public:
    const ResolutionType& resolution() const { return _resolution; }
    ResolutionType& resolution() { return _resolution; }

    Scalar from_user_resolution_unit( const user_resolution_type& u ) { return u; }
    user_resolution_type to_user_resolution_unit( const Scalar& u ) { return u; }
};

}
}

#endif
