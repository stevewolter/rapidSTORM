#ifndef DSTORM_FIT_H
#define DSTORM_FIT_H

#include <math.h>
#include <iostream>
#include <stdexcept>
#include <Eigen/Core>

#include "units/nanolength.h"
#include "input/Traits.h"

#include "localization/Field.h"
#include "localization/Fields.h"

#include <boost/optional/optional.hpp>

namespace dStorm {

class Localization  { 
  public:
#define FIELD(tag,name) \
    localization::Field<tag> name; \
    const localization::Field<tag>& field(tag) const { return name; } \
    localization::Field<tag>& field(tag) { return name; }

    FIELD(localization::PositionX, position_x);
    FIELD(localization::PositionY, position_y);
    FIELD(localization::PositionZ, position_z);
    FIELD(localization::PositionUncertaintyX, position_uncertainty_x);
    FIELD(localization::PositionUncertaintyY, position_uncertainty_y);
    FIELD(localization::PositionUncertaintyZ, position_uncertainty_z);
    FIELD(localization::ImageNumber, frame_number);
    FIELD(localization::Amplitude, amplitude);
    FIELD(localization::PSFWidthX, psf_width_x);
    FIELD(localization::PSFWidthY, psf_width_y);
    FIELD(localization::TwoKernelImprovement, two_kernel_improvement);
    FIELD(localization::FitResidues, fit_residues);
    FIELD(localization::Fluorophore, fluorophore);
    FIELD(localization::LocalBackground, local_background);
    FIELD(localization::CoefficientOfDetermination, coefficient_of_determination);
    FIELD(localization::Molecule, molecule);
#undef FIELD

    Localization();
    Localization( const samplepos& position, localization::Amplitude::ValueType strength );
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

#endif
