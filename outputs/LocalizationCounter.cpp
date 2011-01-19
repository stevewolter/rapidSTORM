#include "LocalizationCounter.h"
#include <dStorm/doc/context.h>

namespace dStorm {
namespace output {

LocalizationCounter::Config::Config()
: simparm::Object("Count", "Count localizations")
{
}

LocalizationCounter::LocalizationCounter(const Config &c)
: OutputObject("LocCountStat", "Localization counting status"),
  count(0),
  last_config_update(0),
  update("LocalizationCount", 
         "Number of localizations found", 0),
  printAtStop(NULL) 
{
    update.helpID = HELP_Count_Count;
}

}
}
