#ifndef TRANSMISSIONFACTORY_H
#define TRANSMISSIONFACTORY_H

#include "Output.h"
#include "Basename.h"
#include <memory>
#include <simparm/Object.hh>
#include <simparm/TreeEntry.hh>
#include "SourceFactory_decl.h"
#include "BasenameAdjustedFileEntry_decl.h"

namespace simparm { class FileEntry; }

namespace dStorm {
namespace output {

/** An OutputSource object is a config that produces
 *  output objects. It is used for forwarding as well
 *  as leaf outputs and controls the config tree and
 *  the basename setting. Implementors must override the
 *  make_output() method and can override the
 *  set_output_file_basename method. */
class OutputSource 
: public simparm::TreeAttributes
{
  private:
    simparm::Node& node;
    OutputSource& operator=(const OutputSource&);
    OutputSource(const OutputSource&);
  protected:
    class AdjustedList;
    std::auto_ptr<AdjustedList> adjustedList;

    OutputSource(simparm::Node& node);
    OutputSource(simparm::Node& node, const OutputSource&);

    /** FileEntry's given to this method will automatically be
     *  updated to the new file basename if it is changed. */
    void adjust_to_basename(BasenameAdjustedFileEntry&);

  public:
    simparm::Node& getNode() { return node; }
    operator simparm::Node&() { return node; }
    const simparm::Node& getNode() const { return node; }
    operator const simparm::Node&() const { return node; }

    virtual ~OutputSource();
    virtual OutputSource* clone() const = 0;

    /** Check whether the Output produced by this OutputSource might 
     *  work, given the maximal additional data provided by the parent
     *  source. */
    virtual void set_source_capabilities( Capabilities ) = 0;
    virtual void set_output_factory(const SourceFactory&) {}
    /** \return A suitable Output object with
     *          all outputs configured. \note The root output
     *          will be constructed last in a tree of
     *          OutputSource objects. */
    virtual std::auto_ptr<Output> make_output() = 0;

    /** Notification for new input file basename. \c avoid_name
     *  is the name of the input file; that name should be avoided,
     *  as it would overwrite our input. */
    virtual void set_output_file_basename
        (const Basename& new_basename);

    virtual std::string getDesc() const = 0;
};

template <typename OutputType>
std::auto_ptr<OutputSource> make_output_source();

}
}

#endif
