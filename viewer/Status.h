#ifndef DSTORM_STATUS_H
#define DSTORM_STATUS_H

#include "viewer/Config.h"
#include "simparm/Entry.h"
#include "simparm/FileEntry.h"
#include "simparm/TriggerEntry.h"
#include <boost/thread/mutex.hpp>
#include "base/Engine.h"

namespace dStorm {
namespace display { class Manager; }
namespace viewer {

class Status {
  public:
    Status(const Config&);
    virtual ~Status();

    Config config;
    simparm::TriggerEntry save;

    boost::mutex mutex;
    dStorm::Engine* engine;

    virtual void adapt_to_changed_config() = 0;
    void attach_ui( simparm::NodeHandle name );
};

}
}

#endif
