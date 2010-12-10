#ifndef DSTORM_GARAGE_H
#define DSTORM_GARAGE_H

#include <dStorm/Localizations.h>
#include <stdexcept>

namespace dStorm {
    class Carburettor;
    class Transmission;
    class TransmissionSourceFactory;
}

class GarageConfig;
class Garage : public simparm::ConfigNode::Callback {
  private:
    std::auto_ptr<dStorm::TransmissionSourceFactory> factory;
    std::auto_ptr<GarageConfig> autoConfig;
    GarageConfig& config;

    /** Initialization code common to all constructors. */
    void init() throw();

    void operator()(simparm::ConfigNode&, Cause, simparm::ConfigNode *) throw();

    void _drive(dStorm::Carburettor *in, dStorm::Transmission *out)
        throw(std::exception);

  public:
    /** Automatic constructor. Constructs a GarageConfig from the
    *  command line arguments provided and either waits on input
    *  or drives a single car from it, depending on the 
    *  presence of the \c --TwiddlerControl parameter. */
    Garage(int argc, char *argv[]) throw();
    Garage(GarageConfig& config) throw();
    virtual ~Garage() throw();

    std::auto_ptr<dStorm::Localizations> drive() throw(std::exception);
    void cruise() throw(std::exception);
};

#endif
