#ifndef TRANSMISSIONFACTORY_H
#define TRANSMISSIONFACTORY_H
#include "Transmission.h"
#include <memory>
#include <ConfigSet.hh>
#include <ConfigEntry.hh>

namespace dStorm {

using namespace simparm;

class TransmissionSource 
: public virtual ConfigObject
{
  protected:
    TransmissionSource() throw() : ConfigObject("", "") {}
    TransmissionSource(const TransmissionSource& o) throw()
        : ConfigObject(o) {}

  public:
    virtual ~TransmissionSource() throw() {}
    virtual TransmissionSource* clone() const = 0;

    virtual std::auto_ptr<Transmission> makeTransmission() throw() = 0;
};

class TransmissionSourceFactory {
  public:
    virtual std::auto_ptr<TransmissionSource> makeTransmissionSource
        (std::string wish) throw() = 0;
};

class TransmissionForwarderSource
: public TransmissionSource
{
  private:
    class InitializationStarter : public ConfigNode::Callback {
        TransmissionForwarderSource* p;
      public:
        InitializationStarter( TransmissionForwarderSource* p ) : p(p) 
        { if (p) p->addChangeCallback( *this ); }
        bool stillActive() const throw() { return p != NULL; }

        void operator() (ConfigNode &src, Cause c, ConfigNode *) {
            if ( p != NULL && c == AddedParent ) {
                src.removeChangeCallback( *this );
                p->initialize_forwarding_deferredly();
                p = NULL;
            }
        }
    };
    InitializationStarter starter;

  protected:
    TransmissionSourceFactory& factory;
    std::auto_ptr< TransmissionSource > output;

    TransmissionForwarderSource(TransmissionSourceFactory& factory) throw() 
    : ConfigObject("", ""), starter( this ), factory(factory) {}
    TransmissionForwarderSource(const TransmissionForwarderSource& o) throw()
        : ConfigObject(o), TransmissionSource(o),
          starter( o.starter.stillActive() ? this : NULL ),
          factory(factory),
          output( (o.output.get() == NULL) ? NULL : o.output->clone() ) 
    {
        if ( output.get() != NULL )
            ConfigNode::push_back( *output );
    }
    ~TransmissionForwarderSource() throw() {}

    virtual void initialize_forwarding_deferredly() throw() {
        output = factory.makeTransmissionSource("");
        ConfigNode::push_back( *output );
    }
};

/** The TransmissionChooser is still virtual because it should be
 *  extended with a class that provides actual entries for the choice. */
class TransmissionChooser 
: public TransmissionSource,
  public ConfigEntryTypedChoice<TransmissionSource>
{
  protected:
    std::list<TransmissionSource*> ordered;
    void registerNamedEntries() throw();

  public:
    TransmissionChooser() throw();
    std::auto_ptr<Transmission> makeTransmission() throw();
    virtual TransmissionChooser* clone() const 
        { return new TransmissionChooser(*this); }
};

}

#endif
