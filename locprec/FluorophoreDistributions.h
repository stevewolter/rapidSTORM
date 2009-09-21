#ifndef LOCPREC_FLUOROPHORE_DISTRIBUTIONS_H
#define LOCPREC_FLUOROPHORE_DISTRIBUTIONS_H

#include "FluorophoreDistribution.h"
#include <simparm/NumericEntry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/Structure.hh>

namespace locprec {
namespace FluorophoreDistributions {

class _Random : public FluorophoreDistribution {
  protected:
    void registerNamedEntries() 
        { push_back( fluorophoreNumber ); }
  public:
    simparm::UnsignedLongEntry fluorophoreNumber;

    _Random();
    _Random* clone() const { return new _Random(*this); }
    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng);
};
typedef simparm::Structure<_Random> Random;

class _Lattice : public FluorophoreDistribution {
  protected:
    void registerNamedEntries() 
        { push_back( latticeDistance ); push_back(border); }
  public:
    simparm::DoubleEntry latticeDistance, border;

    _Lattice();
    _Lattice* clone() const { return new _Lattice(*this); }
    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng);
};
typedef simparm::Structure<_Lattice> Lattice;

class _Lines : public FluorophoreDistribution, 
               public simparm::Node::Callback 
{
  protected:
    void registerNamedEntries();
    void operator()(Node& source_of_event,
                            Cause cause,
                            Node* argument_if_any);
  public:
    class _Line : public simparm::Object {
      protected:
        void registerNamedEntries();
      public:
        simparm::DoubleEntry angle, xoffset, yoffset,
                             density, x_spacing, y_spacing,
                             max_count;
        simparm::UnsignedLongEntry repeat;

        _Line(const std::string& ident);
        Positions fluorophore_positions(const Size& size, gsl_rng* rng)
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
        const Size& size, gsl_rng* rng);
};
typedef simparm::Structure<_Lines> Lines;

}
}

#endif
