#ifndef DSTORM_VIEWER_H
#define DSTORM_VIEWER_H

#include "Backend.h"
#include "Config.h"
#include "Status.h"


#include <dStorm/Localization.h>
#include <boost/thread/mutex.hpp>
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
               public output::Output
{
  public:
    /** Constructor will not display image; this is deferred
        *  until announceStormSize(). */
    Viewer(const Config& config);
    Viewer(const Viewer&);
    virtual ~Viewer();

    AdditionalData announceStormSize(const Announcement &a);
    RunRequirements announce_run(const RunAnnouncement&) ;
    void receiveLocalizations(const EngineResult&);

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames);

  protected:
    void make_new_backend();
    void save_image();
    void change_histogram_normalization_power();
    void change_top_cutoff();

    /** Write the current image into a file. The whole
        *  image is written, regardless of the settings
        *  for viewport or zoom. */
    void writeToFile(const std::string& name);

  private:
    simparm::NodeHandle ui;
    std::auto_ptr< Backend > implementation;
    Output* forwardOutput;
    Engine* repeater;
    boost::optional< Announcement > announcement;
    simparm::BaseAttribute::ConnectionStore listening[4];

    void store_results_( bool job_successful );
    void adapt_to_changed_config();
    void save_density_map();
    void attach_ui_( simparm::NodeHandle );
};

}
}

#endif
