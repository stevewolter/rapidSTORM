#ifndef DSTORM_OUTPUT_BINNING_BINNING_H
#define DSTORM_OUTPUT_BINNING_BINNING_H

#include "binning_decl.h"

#include <dStorm/output/Output.h>
#include <dStorm/traits/image_resolution.h>
#include <boost/units/quantity.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/systems/camera/length.hpp>
#include <dStorm/display/fwd.h>

namespace dStorm {
namespace binning {

struct Unscaled {
    virtual ~Unscaled();
    virtual Unscaled* clone() const = 0;
    virtual void announce(const output::Output::Announcement& a) = 0;
    virtual traits::ImageResolution resolution() const = 0;
    virtual int bin_points( const output::LocalizedImage&, float* target, int stride ) const = 0;
    virtual boost::optional<float> bin_point( const Localization& ) const = 0;
    virtual boost::optional<float> get_uncertainty( const Localization& ) const = 0;

    virtual int field_number() const = 0;
};

struct Scaled
: public Unscaled
{
    virtual ~Scaled() {}
    virtual Scaled* clone() const = 0;
    virtual float get_size() const = 0;
    virtual std::pair< float, float > get_minmax() const = 0;
    virtual double reverse_mapping( float ) const = 0;
    virtual void set_clipping( bool discard_outliers ) = 0;
};

struct UserScaled
: public Scaled
{
    virtual ~UserScaled() {}
    virtual UserScaled* clone() const = 0;
    virtual void set_user_limit( bool lower_limit, const std::string& s ) = 0;
    virtual bool is_bounded() const = 0;
    virtual display::KeyDeclaration key_declaration() const = 0;
};

}
}

#endif
