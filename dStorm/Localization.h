#ifndef DSTORM_FIT_H
#define DSTORM_FIT_H

#include <math.h>
#include <iostream>
#include <Eigen/Core>

#include "units/nanolength.h"
#include "input/Traits.h"

#include "localization/Field.h"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/optional/optional.hpp>
#include <vector>

namespace dStorm {

class Localization  { 
  public:
#define FIELD(type,name) \
    typedef localization::Field<traits::type> type; \
    type name; \
    const localization::Field<traits::type>& field(traits::type tag) const { return name; } \
    localization::Field<traits::type>& field(traits::type tag) { return name; }

    FIELD(PositionX, position_x);
    FIELD(PositionY, position_y);
    FIELD(PositionZ, position_z);
    FIELD(PositionUncertaintyX, position_uncertainty_x);
    FIELD(PositionUncertaintyY, position_uncertainty_y);
    FIELD(PositionUncertaintyZ, position_uncertainty_z);
    FIELD(ImageNumber, frame_number);
    FIELD(Amplitude, amplitude);
    FIELD(PSFWidthX, psf_width_x);
    FIELD(PSFWidthY, psf_width_y);
    FIELD(TwoKernelImprovement, two_kernel_improvement);
    FIELD(FitResidues, fit_residues);
    FIELD(Fluorophore, fluorophore);
    FIELD(LocalBackground, local_background);
#undef FIELD

    struct Fields {
        enum Indices {
            PositionX, PositionUncertaintyX,
            PositionY, PositionUncertaintyY,
            PositionZ, PositionUncertaintyZ,
            ImageNumber, Amplitude,
            PSFWidthX, PSFWidthY,
            TwoKernelImprovement,
            FitResidues,
            Fluorophore,
            LocalBackground,
            Count };
    };
    template <int Index>
    struct Traits {
        typedef typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Index> >::type::Traits type;
    };
    template <typename _Tag, int Index>
    struct Tag {
        typedef typename _Tag::template in< typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Index> >::type::Traits > in;
    };

    typedef std::vector<Localization> Children;
    boost::optional< Children > children;

  private:
    template <typename ConstOrMutLoc> struct _iterator;
  public:
    typedef _iterator<Localization> iterator;
    typedef _iterator<const Localization> const_iterator;
    inline iterator begin();
    inline iterator end();
    inline const_iterator begin() const;
    inline const_iterator end() const;

    Localization();
    Localization( const samplepos& position, Amplitude::Type strength );
    Localization( const Localization& );
    ~Localization();

    const quantity<si::length> psf_width(int index) const {
        return (index == 0) ? psf_width_x() : psf_width_y();
    }
    const quantity<si::length> position_uncertainty(int index) const {
        switch (index) {
            case 0: return position_uncertainty_x();
            case 1: return position_uncertainty_y();
            case 2: return position_uncertainty_z();
            default: throw std::logic_error("Unknown position uncertainty index");
        }
    }
    void set_position_uncertainty(int dimension, quantity<si::length,float> uncertainty) {
        switch (dimension) {
            case 0: position_uncertainty_x = uncertainty; break;
            case 1: position_uncertainty_y = uncertainty; break;
            case 2: position_uncertainty_z = uncertainty; break;
            default: throw std::logic_error("Unknown position uncertainty index");
        }
    }

    const samplepos position() const {
        samplepos rv;
        rv.x() = position_x();
        rv.y() = position_y();
        rv.z() = position_z();
        return rv;
    }

    void set_position(int dimension, samplepos::Scalar value) {
        switch (dimension) {
            case 0: position_x = value; break;
            case 1: position_y = value; break;
            case 2: position_z = value; break;
            default: throw std::logic_error("Unknown position index");
        }
    }
};

std::ostream&
operator<<(std::ostream &o, const Localization& loc);

}

BOOST_FUSION_ADAPT_STRUCT(
    dStorm::Localization,
    (dStorm::Localization::PositionX, position_x)
    (dStorm::Localization::PositionUncertaintyX, position_uncertainty_x)
    (dStorm::Localization::PositionY, position_y)
    (dStorm::Localization::PositionUncertaintyY, position_uncertainty_y)
    (dStorm::Localization::PositionZ, position_z)
    (dStorm::Localization::PositionUncertaintyZ, position_uncertainty_z)
    (dStorm::Localization::ImageNumber, frame_number)
    (dStorm::Localization::Amplitude, amplitude)
    (dStorm::Localization::PSFWidthX, psf_width_x)
    (dStorm::Localization::PSFWidthY, psf_width_y)
    (dStorm::Localization::TwoKernelImprovement, two_kernel_improvement)
    (dStorm::Localization::FitResidues, fit_residues)
    (dStorm::Localization::Fluorophore, fluorophore)
    (dStorm::Localization::LocalBackground, local_background)
)

#endif
