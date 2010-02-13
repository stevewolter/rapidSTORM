#ifndef DSTORM_MODULELOADER_H
#define DSTORM_MODULELOADER_H

#include <boost/utility.hpp>
#include "engine/CarConfig_decl.h"
#include <memory>

namespace dStorm {

class ModuleLoader : boost::noncopyable {
    struct Pimpl;
    friend struct Pimpl;
    std::auto_ptr<Pimpl> pimpl;

    ModuleLoader();
  public:
    static void makeSingleton();
    static ModuleLoader& getSingleton();
    static void destroySingleton();

    void add_modules( engine::CarConfig& );
    std::string makeProgramDescription();
};

}

#endif
