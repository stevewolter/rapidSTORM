#ifndef TESTPLUGIN_BASENAME_PRINTER_H
#define TESTPLUGIN_BASENAME_PRINTER_H

#include <dStorm/output/OutputSource.h>
#include <memory>

std::auto_ptr< dStorm::output::OutputSource > make_basename_printer_source();

#endif
