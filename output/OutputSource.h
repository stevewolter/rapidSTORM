#ifndef TRANSMISSIONFACTORY_H
#define TRANSMISSIONFACTORY_H

#include "output/Output.h"
#include "output/Basename.h"
#include <memory>
#include "simparm/Choice.h"
#include "simparm/TriggerEntry.h"
#include "output/SourceFactory_decl.h"
#include "output/BasenameAdjustedFileEntry_decl.h"
#include "make_clone_allocator.hpp"

namespace simparm { class FileEntry; }

namespace dStorm {
namespace output {

/** An OutputSource object is a config that produces
 *  output objects. It is used for forwarding as well
 *  as leaf outputs and controls the config tree and
 *  the basename setting. Implementors must override the
 *  make_output() method and can override the
 *  set_output_file_basename method. */
class OutputSource : public simparm::Choice
{
  private:
    OutputSource& operator=(const OutputSource&);
    simparm::TriggerEntry destruction;
    simparm::BaseAttribute::ConnectionStore listen;
    void destruction_clicked();
    boost::signals2::signal<void()> destruction_desired;

  protected:
    class AdjustedList;
    std::auto_ptr<AdjustedList> adjustedList;

    OutputSource();
    OutputSource(const OutputSource&);

    void attach_destruction_trigger( simparm::NodeHandle );

    /** FileEntry's given to this method will automatically be
     *  updated to the new file basename if it is changed. */
    void adjust_to_basename(BasenameAdjustedFileEntry&);

  public:
    virtual void hide_in_tree() = 0; 
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
    virtual void attach_full_ui( simparm::NodeHandle ) = 0;
    virtual void attach_ui( simparm::NodeHandle ) = 0;
    virtual boost::signals2::connection notify_when_destruction_is_desired(
        boost::signals2::slot<void()> callback );
};

}
}

DSTORM_MAKE_BOOST_CLONE_ALLOCATOR( dStorm::output::OutputSource )

#endif
