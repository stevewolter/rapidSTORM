#include "ProgressEntry.hh"
#include <math.h>

namespace simparm {

class ProgressEntry::ASCII_Progress_Shower
  : public Node::Callback
{
  private:
    const Attribute<double>& value;
    std::ostream &asciiProgress;
    int asciiProgressLevel;

  public:
    ASCII_Progress_Shower( 
        Attribute<double>& val, 
        std::ostream& out ) 
    : Node::Callback( Event::ValueChanged ),
      value(val),
      asciiProgress(out), asciiProgressLevel(0)
    { val.addChangeCallback(*this); }
    ~ASCII_Progress_Shower() { 
        if (asciiProgressLevel < 100 && asciiProgressLevel > 0)
            asciiProgress << std::endl;
    }
    void operator()(const Event&) {
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
    std::ostream& getStream() { return asciiProgress; }
};

ProgressEntry::ProgressEntry(const ProgressEntry &entry)
: Entry<double>(entry), indeterminate(entry.indeterminate)
{
    this->increment = 0.01;
    if ( entry.display.get() != NULL )
        display.reset( 
            new ASCII_Progress_Shower( value, entry.display->getStream()));
    push_back( indeterminate );
}

ProgressEntry::ProgressEntry(string name, string desc, double value)
: Entry<double>(name, desc, value), indeterminate("indeterminate", false)
{
   min = (0);
   max = (1+1E-10);
   increment = 0.01;
    push_back( indeterminate );
}

ProgressEntry::~ProgressEntry() {}

void ProgressEntry::makeASCIIBar(ostream &o) 
   { display.reset( new ASCII_Progress_Shower( value, o ) ); }
void ProgressEntry::stopASCIIBar() 
   { display.reset( NULL); }

}
