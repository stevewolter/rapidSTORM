#ifndef DSTORM_VIEWER_H
#define DSTORM_VIEWER_H

#include "Backend.h"
#include "Config.h"
#include "Status.h"

#include <dStorm/Localization.h>
#include <boost/thread/mutex.hpp>
#include <dStorm/outputs/LocalizationList.h>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <Eigen/Core>

namespace dStorm {
namespace viewer {

/** The Viewer class collects fits into a BinnedLocalizations
*  image, normalizes the resulting image and shows a part
*  of that image in a window. */
class Viewer : public Status,
               public output::OutputObject,
               public simparm::Node::Callback
{
  public:
    typedef viewer::Config Config;

    /** Constructor will not display image; this is deferred
        *  until announceStormSize(). */
    Viewer(const Config& config);
    Viewer(const Viewer&);
    virtual ~Viewer();
    Viewer* clone() const 
        { throw std::runtime_error("No Viewer::clone()"); }

    AdditionalData announceStormSize(const Announcement &a);
    RunRequirements announce_run(const RunAnnouncement&) ;
    void receiveLocalizations(const EngineResult&);
    void store_results();

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames);

  protected:
    /** Configuration event received. */
    void operator()(const simparm::Event&);

    /** Write the current image into a file. The whole
        *  image is written, regardless of the settings
        *  for viewport or zoom. */
    void writeToFile(const std::string& name);

  private:
    boost::recursive_mutex *output_mutex;
    std::auto_ptr< Backend > implementation;
    Output* forwardOutput;
    Engine* repeater;
    boost::optional< Announcement > announcement;

    void adapt_to_changed_config();
};

}
}

#endif
