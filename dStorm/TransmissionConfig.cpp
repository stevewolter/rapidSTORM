#include "TransmissionConfig.h"

namespace dStorm {

#define NAME "OutputMethod"
#define DESC "Select output method"
TransmissionChooser::TransmissionChooser() throw() 
: ConfigObject(NAME, DESC),
  ConfigEntryTypedChoice<TransmissionSource>(NAME, DESC)
{
    registerNamedEntries();
}

void TransmissionChooser::registerNamedEntries() throw() {}

std::auto_ptr<Transmission> TransmissionChooser::makeTransmission() throw() {
    if ( this->isValid() )
        return getCurrentChoice().makeTransmission();
    else
        return std::auto_ptr<Transmission>(NULL);
}

}
