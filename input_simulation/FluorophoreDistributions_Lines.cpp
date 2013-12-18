#include "FluorophoreDistributions.h"

#include <boost/math/constants/constants.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/lexical_cast.hpp>

#include <Eigen/Core>
#include <boost/units/Eigen/Array>

using namespace Eigen;
using namespace boost::units;

namespace input_simulation {
namespace FluorophoreDistributions {

using namespace boost::units;

class Lines : public FluorophoreDistribution
{
  protected:
    void attach_ui( simparm::NodeHandle at ) ;
    void commit_line_count();
  public:
    class Line {
        simparm::Object name_object;
      public:
        dStorm::NanometreEntry xoffset, yoffset, zoffset,
            density, x_spacing, y_spacing, z_spacing;
        simparm::Entry<double> angle, z_angle, max_count;
        simparm::Entry<unsigned long> repeat;

        Line(const std::string& ident);
        Positions fluorophore_positions(const Size& size, gsl_rng* rng) const;
        void attach_ui( simparm::NodeHandle );
    };

  private:
    boost::ptr_vector<Line> lines;
    simparm::NodeHandle current_ui;
    simparm::BaseAttribute::ConnectionStore listening;

  public:
    simparm::Entry<unsigned int> line_count;

    Lines();
    Lines(const Lines&);
    ~Lines();
    Lines& operator=(const Lines&) { throw std::logic_error("No assignment operator."); }
    Lines* clone() const { return new Lines(*this); }
    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng) const;
};

void Lines::Line::attach_ui( simparm::NodeHandle t ) {
    simparm::NodeHandle r = name_object.attach_ui(t);
    xoffset.attach_ui( r );
    yoffset.attach_ui( r );
    zoffset.attach_ui( r );
    angle.attach_ui( r );
    z_angle.attach_ui( r );
    density.attach_ui( r );
    x_spacing.attach_ui( r );
    y_spacing.attach_ui( r );
    z_spacing.attach_ui( r );
    max_count.attach_ui( r );
    repeat.attach_ui( r );
}

Lines::Lines()
: FluorophoreDistribution("Lines", "Fluorophores on lines"),
  line_count("LineCount", "Number of line sets", 1)
{
    commit_line_count();
}

Lines::Lines(const Lines& c)
: FluorophoreDistribution(c),
  lines( c.lines ),
  line_count( c.line_count )
{
}

Lines::~Lines() { }

Lines::Line::Line(const std::string& ident) 
: name_object("Line" + ident, "Line object " + ident),
  xoffset("StartX", "X start position of line", 1E2 * si::nanometer),
  yoffset("StartY", "Y start position of line", 1E2 * si::nanometer),
  zoffset("StartZ", "Z start position of line", 0 * si::nanometer),
  density("Density", "Distance between fluorophores on line", 40 * si::nanometer),
  x_spacing("XSpacing", "Offset each repeated line in X by", 0 * si::nanometer),
  y_spacing("YSpacing", "Offset each repeated line in Y by", 500 * si::nanometer),
  z_spacing("ZSpacing", "Offset each repeated line in Z by", 0 * si::nanometer),
  angle("Angle", "Line angle in XY plane in degree", 0),
  z_angle("AngleZ", "Line angle to XY plane in degree", 0),
  max_count("MaxPerLine", "Maximal number of fluorophores per line", 0),
  repeat("Repeat", "Repeat line n times", 1)
{
    angle.min = (0);
    angle.max = (360);
    z_angle.min = (-90);
    z_angle.max = (90);
}

void Lines::attach_ui( simparm::NodeHandle at ) {
    listening = line_count.value.notify_on_value_change( 
        boost::bind( &Lines::commit_line_count, this ) );

    current_ui = attach_parent( at );
    line_count.attach_ui( current_ui );
    for (boost::ptr_vector<Line>::iterator i = lines.begin(); i != lines.end(); i++)
        i->attach_ui( current_ui );
}

FluorophoreDistribution::Positions Lines::fluorophore_positions(
    const Size& size,
    gsl_rng* rng
) const {
    Positions rv;
    for (boost::ptr_vector<Line>::const_iterator
         i = lines.begin(); i != lines.end(); i++)
    {
        Positions sub = i->fluorophore_positions(size, rng);
        while ( !sub.empty() ) {
            rv.push( sub.front() );
            sub.pop();
        }
    }
    return rv;
}

FluorophoreDistribution::Positions Lines::Line::
    fluorophore_positions( const Size& size, gsl_rng*) const
{
    Positions rv;

    Fluorophore::Position zero = Fluorophore::Position::Zero(),
          border = size, shift;
    zero.z() = Fluorophore::Position::Scalar::from_value(-std::numeric_limits<float>::infinity());
    border.z() = Fluorophore::Position::Scalar::from_value(std::numeric_limits<float>::infinity());
    const double pi = boost::math::constants::pi<double>();
    shift.x() = Fluorophore::Position::Scalar(cos(2*pi*angle()/360)*cos(2*pi*z_angle()/360) * density()); 
    shift.y() = Fluorophore::Position::Scalar(sin(2*pi*angle()/360)*cos(2*pi*z_angle()/360) * density());
    shift.z() = Fluorophore::Position::Scalar(sin(2*pi*z_angle()/360) * density()); 

    int number = 0;
    for (unsigned int i = 0; i <= repeat(); i++) {
        Fluorophore::Position start;
        start.x() = Fluorophore::Position::Scalar(xoffset() + double(i)*x_spacing());
        start.y() = Fluorophore::Position::Scalar(yoffset() + double(i)*y_spacing());
        start.z() = Fluorophore::Position::Scalar(zoffset() + double(i)*z_spacing());
        for(
            Fluorophore::Position p = start;
            (zero.array() <= p.array()).all() && (p.array() < border.array()).all() && (max_count() == 0 || number < max_count());
            p += shift
        ) { rv.push(p); number++; }
        for(
            Fluorophore::Position p = start - shift;
            (zero.array() <= p.array()).all() && (p.array() < border.array()).all() && (max_count() == 0 || number < max_count());
            p -= shift
        ) { rv.push(p); number++; }
    }

    return rv;
}

void Lines::commit_line_count()
{
    size_t count = line_count();
    while ( count > lines.size() ) {
        lines.push_back( new Line( boost::lexical_cast<std::string>( lines.size() ) ) );
        if ( current_ui )
            lines.back().attach_ui( current_ui );
    }
    while ( count < lines.size() )
        lines.pop_back();
}

std::auto_ptr< FluorophoreDistribution > make_lines() {
    return std::auto_ptr< FluorophoreDistribution >( new Lines() );
}

}
}
