#include "LocalizationCounter.h"

namespace dStorm {
namespace output {

LocalizationCounter::_Config::_Config()
: simparm::Object("Count", "Count localizations"),
  output_file("ToFile", "Write localization count to file")
{
    output_file.userLevel = Object::Intermediate;
}

LocalizationCounter::LocalizationCounter(const Config &c)
: OutputObject("LocCountStat", "Localization counting status"),
  count(0),
  last_config_update(0),
  update("LocalizationCount", 
         "Number of localizations found", 0)
{
    update.helpID = "#Count_Count";
    print_count.reset( new std::ofstream( c.output_file().c_str(), std::ios::out ) );
}

}
}
