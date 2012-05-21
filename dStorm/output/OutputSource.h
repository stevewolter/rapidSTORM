#ifndef TRANSMISSIONFACTORY_H
#define TRANSMISSIONFACTORY_H

#include "Output.h"
#include "Basename.h"
#include <memory>
#include <simparm/Object.hh>
#include <simparm/TreeEntry.hh>
#include "SourceFactory_decl.h"
#include "BasenameAdjustedFileEntry_decl.h"
#include <dStorm/make_clone_allocator.hpp>

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
{
  private:
    simparm::TreeAttributes tree_attributes;
    OutputSource& operator=(const OutputSource&);
  protected:
    class AdjustedList;
    std::auto_ptr<AdjustedList> adjustedList;

    OutputSource();
    OutputSource(const OutputSource&);

    /** FileEntry's given to this method will automatically be
     *  updated to the new file basename if it is changed. */
    void adjust_to_basename(BasenameAdjustedFileEntry&);

    void attach_source_ui( simparm::Node& at )
        { tree_attributes.registerNamedEntries( at ); }

  public:
    void hide_in_tree() { tree_attributes.show_in_tree = false; }
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

    virtual std::string getName() const = 0;
    virtual std::string getDesc() const = 0;
    virtual void attach_full_ui( simparm::Node& ) = 0;
    virtual void attach_ui( simparm::Node& ) = 0;
};

}
}

DSTORM_MAKE_BOOST_CLONE_ALLOCATOR( dStorm::output::OutputSource )

#endif
