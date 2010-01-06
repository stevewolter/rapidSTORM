#ifndef DSTORM_ADC_H
#define DSTORM_ADC_H

#include <boost/units/unit.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/base_dimension.hpp>
#include <boost/units/base_unit.hpp>
#include <boost/units/base_units/si/meter.hpp>
#include <boost/units/physical_dimensions/area.hpp>
#include <boost/units/physical_dimensions/velocity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/time.hpp>

namespace dStorm {
namespace camera {

struct intensity_base_dimension 
    : public boost::units::base_dimension<intensity_base_dimension,1>
    {};
typedef intensity_base_dimension::dimension_type
    intensity_dimension;

struct intensity_base_unit 
    : public boost::units::base_unit<intensity_base_unit, 
                    intensity_dimension, 1> 
{};

struct pixel_base_unit 
    : public boost::units::base_unit<pixel_base_unit, 
                    boost::units::length_dimension, 2> 
    {};
struct time_base_unit 
    : public boost::units::base_unit<time_base_unit, 
                    boost::units::time_dimension, 3> 
    {};

typedef boost::units::make_system<
    intensity_base_unit, pixel_base_unit, time_base_unit>::type
    system;

typedef boost::units::unit<
    intensity_dimension, system> intensity;
typedef boost::units::unit<
    boost::units::length_dimension, system> length;
typedef boost::units::unit<
    boost::units::time_dimension, system> time;
typedef boost::units::unit<
    boost::units::area_dimension, system> area;
typedef boost::units::unit< 
    boost::units::velocity_dimension, system > velocity;

static const length pixel;
static const length pixels;
static const time frame;
static const time frames;
static const intensity ad_count;
static const intensity ad_counts;

typedef boost::units::divide_typeof_helper< 
    boost::units::si::length, length >::type pixel_size;
typedef boost::units::divide_typeof_helper< 
    boost::units::si::time, time >::type frame_rate;

}
}

namespace boost {
namespace units {

template<> 
struct base_unit_info<dStorm::camera::intensity_base_unit>
{
    static std::string name() { return "A/D count"; }
    static std::string symbol() { return "ADC"; }
};

template<> 
struct base_unit_info<dStorm::camera::time_base_unit>
{
    static std::string name() { return "frame"; }
    static std::string symbol() { return "fr"; }
};

template<> 
struct base_unit_info<dStorm::camera::pixel_base_unit>
{
    static std::string name() { return "pixel"; }
    static std::string symbol() { return "px"; }
};

}
}


#endif
