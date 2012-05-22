#ifndef LOCPREC_FLUOROPHORE_DISTRIBUTIONS_H
#define LOCPREC_FLUOROPHORE_DISTRIBUTIONS_H

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include "FluorophoreDistribution.h"
#include <simparm/Entry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/NodeHandle.hh>
#include <dStorm/UnitEntries.h>

namespace input_simulation {
namespace FluorophoreDistributions {

using namespace boost::units;

class Random : public FluorophoreDistribution {
  protected:
    void attach_ui( simparm::Node& at ) 
        { fluorophoreNumber.attach_ui( attach_parent( at ) ); }
  public:
    simparm::Entry<unsigned long> fluorophoreNumber;

    Random();
    Random* clone() const { return new Random(*this); }
    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng) const;
};

std::auto_ptr< FluorophoreDistribution > make_lattice();

class Lines : public FluorophoreDistribution, 
               public simparm::Listener 
{
  protected:
    void attach_ui( simparm::Node& at ) ;
    void operator()(const simparm::Event&);
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

}
}

#endif
