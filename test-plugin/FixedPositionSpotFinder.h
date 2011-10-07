#ifndef FIXED_POSITION_SPOT_FINDER_H
#define FIXED_POSITION_SPOT_FINDER_H

#include <simparm/Object.hh>
#include <simparm/Structure.hh>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/SpotFinder.h>

namespace FixedPositionSpotFinder {

class Finder : public dStorm::engine::spot_finder::Base {
    class _Config;
  public:
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::engine::spot_finder::Builder<Finder> Factory;

    inline Finder (const Config& myconf,
                        const dStorm::engine::spot_finder::Job &);
    ~Finder() {}
    Finder* clone() const { return new Finder(*this); }

    void smooth( const dStorm::engine::Image2D & ) {}
};

class Finder::_Config : public simparm::Object
{
  protected:
    void registerNamedEntries() { push_back(x); push_back(y); }
    public:
    simparm::Entry<unsigned long> x, y;

    _Config() : simparm::Object("FixedPosition", "Find at fixed position"), x("XSinglePixel", "Position to find spots at [x]"), y("YSinglePixel", "Position to find spots at [y]") {}
};


Finder::Finder (const Config& myconf,
                    const dStorm::engine::spot_finder::Job& job)
: Base(job) { this->smoothed.fill(0); this->smoothed(myconf.x(), myconf.y()) = 1000; }

}

#endif
