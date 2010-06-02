#ifndef DSTORM_STATUS_H
#define DSTORM_STATUS_H

#include "Status_decl.h"
#include "Config_decl.h"
#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/Structure.hh>
#include <dStorm/helpers/thread.h>

namespace dStorm {
namespace viewer {

struct Status {
    Status(const Config&);
    ~Status();

    simparm::TriggerEntry reshow_output;
    simparm::FileEntry tifFile;
    simparm::BoolEntry save_with_key;
    simparm::DoubleEntry resolutionEnhancement, histogramPower;
    simparm::TriggerEntry save;

    ost::Mutex mutex;

    virtual void adapt_to_changed_config() = 0;
};

}
}

#endif
