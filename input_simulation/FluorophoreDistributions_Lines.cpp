#include "FluorophoreDistributions.h"
#include <Eigen/Core>
#include <boost/units/Eigen/Array>

using namespace Eigen;
using namespace boost::units;

namespace input_simulation {
namespace FluorophoreDistributions {

using namespace boost::units;

_Lines::_Lines()
: FluorophoreDistribution("Lines", "Fluorophores on lines"),
  simparm::Node::Callback(simparm::Event::ValueChanged),
  addLine("AddLine", "Add new line set"),
  removeLine("RemoveLine", "Remove selected line")
{
    receive_changes_from( addLine.value );
    receive_changes_from( removeLine.value );

    addLine.trigger();
}

_Lines::_Lines(const _Lines& c)
: FluorophoreDistribution(c),
  simparm::Node::Callback(simparm::Event::ValueChanged),
  addLine("AddLine", "Add new line set"),
  removeLine("RemoveLine", "Remove selected line")
{
    receive_changes_from( addLine.value );
    receive_changes_from( removeLine.value );

    lines.resize( c.lines.size(), NULL );
    for (unsigned int i = 0; i < c.lines.size(); i++)
        if ( c.lines[i] != NULL ) {
            lines[i] = new Line(*c.lines[i]);
            push_back( *lines[i] );
        }
}

_Lines::~_Lines()
{
    for (unsigned int i = 0; i < lines.size(); i++) {
        delete lines[i];
        lines[i] = NULL;
    }
}

_Lines::_Line::_Line(const std::string& ident) 
: simparm::Object("Line" + ident, "Line object " + ident),
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

void _Lines::registerNamedEntries() {
    push_back( addLine );
    push_back( removeLine );
}

void _Lines::_Line::registerNamedEntries() {
    push_back( xoffset );
    push_back( yoffset );
    push_back( zoffset );
    push_back( angle );
    push_back( z_angle );
    push_back( density );
    push_back( x_spacing );
    push_back( y_spacing );
    push_back( z_spacing );
    push_back( max_count );
    push_back( repeat );
}

FluorophoreDistribution::Positions _Lines::fluorophore_positions(
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

FluorophoreDistribution::Positions _Lines::_Line::
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

void _Lines::operator()(const simparm::Event& e) 
{
    if ( &e.source == &addLine.value && addLine.triggered() ) {
        addLine.untrigger();

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

        push_back( *line );
    } else if ( &e.source == &removeLine.value && removeLine.triggered() ) {
        removeLine.untrigger();
        delete lines.back();
        lines.back() = NULL;
    }
}

}
}
