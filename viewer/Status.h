#ifndef DSTORM_STATUS_H
#define DSTORM_STATUS_H

#include "Status_decl.h"
#include "Config_decl.h"
#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/Structure.hh>
#include <boost/thread/mutex.hpp>

namespace dStorm {
namespace display { class Manager; }
namespace viewer {

struct Status {
    Status(const Config&);
    ~Status();

    simparm::TriggerEntry reshow_output;
    simparm::FileEntry tifFile;
    simparm::BoolEntry save_with_key;
    simparm::Entry<double> histogramPower;
    simparm::TriggerEntry save;

    boost::mutex mutex;
    display::Manager *manager;

    virtual void adapt_to_changed_config() = 0;
};

}
}

#endif
