#ifndef LOCPREC_FLUOROPHORE_DISTRIBUTIONS_H
#define LOCPREC_FLUOROPHORE_DISTRIBUTIONS_H

#include "FluorophoreDistribution.h"
#include <simparm/Entry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/Structure.hh>
#include <dStorm/UnitEntries.h>

namespace locprec {
namespace FluorophoreDistributions {

using namespace boost::units;

class _Random : public FluorophoreDistribution {
  protected:
    void registerNamedEntries() 
        { push_back( fluorophoreNumber ); }
  public:
    simparm::Entry<unsigned long> fluorophoreNumber;

    _Random();
    _Random* clone() const { return new _Random(*this); }
    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng) const;
};
typedef simparm::Structure<_Random> Random;

class _Lattice : public FluorophoreDistribution {
  protected:
    void registerNamedEntries() 
        { push_back( latticeDistance ); push_back(border); }
  public:
    dStorm::NanometreEntry latticeDistance, border;

    _Lattice();
    _Lattice* clone() const { return new _Lattice(*this); }
    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng) const;
};
typedef simparm::Structure<_Lattice> Lattice;

class _Lines : public FluorophoreDistribution, 
               public simparm::Node::Callback 
{
  protected:
    void registerNamedEntries();
    void operator()(const simparm::Event&);
  public:
    class _Line : public simparm::Object {
      protected:
        void registerNamedEntries();
      public:
        dStorm::NanometreEntry xoffset, yoffset, zoffset,
            density, x_spacing, y_spacing, z_spacing;
        simparm::Entry<double> angle, z_angle, max_count;
        simparm::Entry<unsigned long> repeat;

        _Line(const std::string& ident);
        Positions fluorophore_positions(const Size& size, gsl_rng* rng) const
;
    };
    typedef simparm::Structure<_Line> Line;

  private:
    std::vector<Line*> lines;

  public:
    simparm::ChoiceEntry lineRemoval;
    simparm::TriggerEntry addLine, removeLine;

    _Lines();
    _Lines(const _Lines&);
    ~_Lines();
    _Lines& operator=(const _Lines&) { throw std::logic_error("No assignment operator."); }
    _Lines* clone() const { return new _Lines(*this); }
    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng) const;
};
typedef simparm::Structure<_Lines> Lines;

}
}

#endif
