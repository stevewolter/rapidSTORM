#ifndef DSTORM_GARAGE_H
#define DSTORM_GARAGE_H

#include <simparm/Node.hh>
#include <dStorm/output/Localizations.h>
#include <stdexcept>

namespace dStorm {
    class Carburettor;
}

class GarageConfig;
class ModuleHandler;
class Garage : public simparm::Node::Callback {
  private:
    std::auto_ptr<ModuleHandler> moduleHandler;
    std::auto_ptr<GarageConfig> autoConfig;
    GarageConfig& config;

    /** Initialization code common to all constructors. */
    void init() throw();

    void operator()(simparm::Node&, Cause, simparm::Node *) throw();

    void _drive();

  public:
    /** Automatic constructor. Constructs a GarageConfig from the
    *  command line arguments provided and either waits on input
    *  or drives a single car from it, depending on the 
    *  presence of the \c --TwiddlerControl parameter. */
    Garage(int argc, char *argv[]);
    Garage(GarageConfig& config) throw();
    virtual ~Garage() throw();

    std::auto_ptr<dStorm::Localizations> drive() throw(std::exception);
    void cruise() throw(std::exception);
};

#endif
