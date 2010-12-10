#include "ConfigEntry.hh"
#include <math.h>

namespace simparm {

class ConfigEntryProgress::ASCII_Progress_Shower
  : public ConfigNode::Callback
{
  private:
    const ConfigAttribute<double>& value;
    std::ostream &asciiProgress;
    int asciiProgressLevel;

  public:
    ASCII_Progress_Shower( 
        ConfigAttribute<double>& val, 
        std::ostream& out ) 
    : value(val),
      asciiProgress(out), asciiProgressLevel(0)
    { val.addChangeCallback(*this); }
    ~ASCII_Progress_Shower() throw() { 
        if (asciiProgressLevel < 100 && asciiProgressLevel > 0)
            asciiProgress << std::endl;
    }
    void operator()(ConfigNode &, Cause cause, ConfigNode *) throw() {
        if ( cause != ValueChanged ) return;
        int discrete = round(100 * value());
        if (asciiProgressLevel > discrete) {
            asciiProgress << "\r";
            asciiProgressLevel = 0;
        }
        while (asciiProgressLevel < discrete) {
            asciiProgressLevel++;
            if (asciiProgressLevel % 10 == 0)
                asciiProgress << " " << asciiProgressLevel << " ";
            else if (asciiProgressLevel % 2 == 0)
                asciiProgress << ':';

            if (asciiProgressLevel == 100)
                asciiProgress << std::endl;
        }
    }
    std::ostream& getStream() throw() { return asciiProgress; }
};

ConfigEntryProgress::ConfigEntryProgress(const ConfigEntryProgress &entry)
: ConfigObject(entry), ConfigEntryDouble(entry)
{
    if ( entry.display.get() != NULL )
        display.reset( 
            new ASCII_Progress_Shower( value, entry.display->getStream()));
}

ConfigEntryProgress::ConfigEntryProgress(string name, string desc,
    double value, double increment)
: ConfigObject(name, desc),
  ConfigEntryDouble(name, desc, value, increment)
{
   setMin(0);
   setMax(1);
}

ConfigEntryProgress::~ConfigEntryProgress() throw() {}

void ConfigEntryProgress::makeASCIIBar(ostream &o) 
   { display.reset( new ASCII_Progress_Shower( value, o ) ); }
void ConfigEntryProgress::stopASCIIBar() 
   { display.reset( NULL); }

}
