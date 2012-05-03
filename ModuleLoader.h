#ifndef DSTORM_MODULELOADER_H
#define DSTORM_MODULELOADER_H

#include <boost/utility.hpp>
#include <dStorm/Config_decl.h>
#include <memory>
#include <dStorm/JobMaster.h>

namespace dStorm {

void add_modules( Config& );
std::string makeProgramDescription();

}

#endif
