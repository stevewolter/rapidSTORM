#include "LocalizationCounter.h"
#include "help_context.h"

namespace dStorm {

LocalizationCounter::Config::Config()
: simparm::Object("Count", "Count localizations")
{
}

LocalizationCounter::LocalizationCounter(const Config &)
: Object("LocCountStat", "Localization counting status"),
  count(0),
  last_config_update(0),
  update("LocalizationCount", 
         "Number of localizations found", 0),
  printAtStop(NULL) 
{
    update.helpID = HELP_Count_Count;
}

};
