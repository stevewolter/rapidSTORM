#include <ConfigEntry.hh>
#include <ConfigEventHandler.hh>
#include <functional>

class ConditionalEnabler : public ConfigEventHandler {
  public:
    ConditionalEnabler(const ConfigEntry& dependsOn, 
};
