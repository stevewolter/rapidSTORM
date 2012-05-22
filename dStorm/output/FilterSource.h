#ifndef DSTORM_BASETRANSMISSIONCONFIGS
#define DSTORM_BASETRANSMISSIONCONFIGS

#include <simparm/TriggerEntry.hh>
#include <simparm/ManagedChoiceEntry.hh>
#include <simparm/NodeHandle.hh>
#include "OutputSource.h"
#include <map>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/function/function1.hpp>

namespace dStorm {
namespace output {

class SourceFactory;

/** The FilterSource class is the base class for all
 *  OutputSource objects which have other outputs as
 *  output. For example, see the LocalizationFilter. 
 *
 *  The FilterSource is built around an SourceFactory, which is added
 *  to its simparm::Node *  element and delivers the OutputSource child
 *  object. To avoid initialization loops, this factory is not constructed
 *  in the normal constructor, but rather via initialize() or in the
 *  copy constructor.
 *
 *  This class will transparently add a Crankshaft transmission to the
 *  output when multiple outputs are given. If this is the
 *  case, it has to distinguish between several config items
 *  with the same name, for example multiple TableOutput configs.
 *  It does so by packing each node into a transparent node called
 *  CrankshaftNodeX, where X is a unique integer.
 *
 *  The important methods and members are output, which is the output
 *  transmission that should be used by deriving classes, and add(),
 *  which can be used to explicitly set the output member. 
 **/
class FilterSource
: public OutputSource,
  public simparm::Listener
{
  private:
    /** The unique integer X for the next disambiguation node. */
    int next_identity;

    /** The basename is saved for freshly constructed entries. */
    Basename basename;

    /** The factory is the source object for a new transmission source.
     *  This new transmission source is fetched via 
     *  factory->make_transmission().
     *
     *  If the initialization is delayed via OnCopy, the factory member
     *  will not be set until initialize() is called. This avoids init
     *  loops. */
    std::auto_ptr<SourceFactory> factory;
    simparm::NodeHandle my_node;

    struct Suboutput;
    typedef simparm::ManagedChoiceEntry<Suboutput> SuboutputChoice;
    SuboutputChoice suboutputs;

    /** \return true after initialize() was called. */
    bool is_initialized() const { return factory.get() != NULL; }
    /** \see simparm::Listener::operator() */
    void operator()(const simparm::Event&);
    /** Construct the disambiguation node for the given transmission,
     *  add the remover entry and insert it into our config node. */
    void link_transmission( OutputSource* src );

    void add_new_element();

  protected:
    FilterSource();
    FilterSource( const FilterSource& );

    SourceFactory* getFactory() { return factory.get(); }
    const SourceFactory* getFactory() const { return factory.get(); }

    void attach_children_ui( simparm::Node& );

  public:
    ~FilterSource();
    FilterSource* clone() const = 0;

    /** This method triggers the delayed initialization of the factory
     *  element. */
    virtual void set_output_factory(const SourceFactory& f);

    virtual void set_source_capabilities( Capabilities );

    /** Explicitely set the output element. Circumvents the 
     *  \c factory. */
    void add(std::auto_ptr<OutputSource> src);
    /** Convenience wrapper for the auto_ptr-add */
    void add(OutputSource *src)
        { add( std::auto_ptr<OutputSource>(src) ); }

    /** \see dStorm::OutputSource */
    void set_output_file_basename(const Basename& basename);

    void for_each_suboutput( boost::function1<void,const OutputSource&> f ) const;

    std::auto_ptr<Output> make_output(); 
};

typedef FilterSource TransmissionSource;

}
}

#endif
