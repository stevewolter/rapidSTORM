#ifndef DSTORM_GUF_PSF_DERIVABLE_H
#define DSTORM_GUF_PSF_DERIVABLE_H

namespace dStorm {
namespace guf {
namespace PSF {

struct Derivable {
    template <typename Parameter>
    struct apply {
        typedef boost::mpl::bool_<true> type;
    };
};
template <> struct Derivable::apply< dStorm::guf::PSF::ZPosition > {
        typedef boost::mpl::bool_<false> type;
};

}
}
}

#endif
