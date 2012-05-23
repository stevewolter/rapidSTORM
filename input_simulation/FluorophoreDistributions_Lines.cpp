#include "FluorophoreDistributions.h"
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
    void attach_ui( simparm::Node& at ) ;
    void add_line();
    void add_line_trigger();
    void remove_line();
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
        void attach_ui( simparm::Node& );
    };

  private:
    std::vector<Line*> lines;
    simparm::NodeHandle current_ui;
    simparm::BaseAttribute::ConnectionStore listening[2];

  public:
    simparm::TriggerEntry addLine, removeLine;

    Lines();
    Lines(const Lines&);
    ~Lines();
    Lines& operator=(const Lines&) { throw std::logic_error("No assignment operator."); }
    Lines* clone() const { return new Lines(*this); }
    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng) const;
};

void Lines::Line::attach_ui( simparm::Node& t ) {
    simparm::NodeRef r = name_object.attach_ui(t);
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
  addLine("AddLine", "Add new line set"),
  removeLine("RemoveLine", "Remove selected line")
{
    add_line();
}

Lines::Lines(const Lines& c)
: FluorophoreDistribution(c),
  addLine("AddLine", "Add new line set"),
  removeLine("RemoveLine", "Remove selected line")
{
    lines.resize( c.lines.size(), NULL );
    for (unsigned int i = 0; i < c.lines.size(); i++)
        if ( c.lines[i] != NULL ) {
            lines[i] = new Line(*c.lines[i]);
            if ( current_ui )
                lines[i]->attach_ui( *current_ui );
        }
}

Lines::~Lines()
{
    for (unsigned int i = 0; i < lines.size(); i++) {
        delete lines[i];
        lines[i] = NULL;
    }
}

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

void Lines::attach_ui( simparm::Node& at ) {
    listening[0] = addLine.value.notify_on_value_change( 
        boost::bind( &Lines::add_line_trigger, this ) );
    listening[1] = addLine.value.notify_on_value_change( 
        boost::bind( &Lines::remove_line, this ) );

    current_ui = attach_parent( at );
    addLine.attach_ui( *current_ui );
    removeLine.attach_ui( *current_ui );
    for (std::vector<Line*>::const_iterator i = lines.begin(); i != lines.end(); i++)
        (*i)->attach_ui( *current_ui );
}

FluorophoreDistribution::Positions Lines::fluorophore_positions(
    const Size& size,
    gsl_rng* rng
) const {
    Positions rv;
    for (std::vector<Line*>::const_iterator
         i = lines.begin(); i != lines.end(); i++)
    {
        Positions sub = (*i)->fluorophore_positions(size, rng);
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
    shift.x() = Fluorophore::Position::Scalar(cos(2*M_PI*angle()/360)*cos(2*M_PI*z_angle()/360) * density()); 
    shift.y() = Fluorophore::Position::Scalar(sin(2*M_PI*angle()/360)*cos(2*M_PI*z_angle()/360) * density());
    shift.z() = Fluorophore::Position::Scalar(sin(2*M_PI*z_angle()/360) * density()); 

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

void Lines::add_line_trigger()
{
    if ( addLine.triggered() ) {
        addLine.untrigger();
        add_line();
    }
}

void Lines::add_line()
{
    unsigned int insert = lines.size();
    for (unsigned int i = 0; i < lines.size(); i++)
        if ( lines[i] == NULL ) 
            { insert = i; break; }

    std::stringstream ident;
    ident << insert;
    Line *line = new Line(ident.str());
    if ( insert < lines.size() )
        lines[insert] = line;
    else
        lines.push_back( line );

    if ( current_ui )
        line->attach_ui( *current_ui );
}

void Lines::remove_line() {
    if ( removeLine.triggered() ) {
        removeLine.untrigger();
        delete lines.back();
        lines.back() = NULL;
    }
}

std::auto_ptr< FluorophoreDistribution > make_lines() {
    return std::auto_ptr< FluorophoreDistribution >( new Lines() );
}

}
}
