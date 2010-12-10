#ifndef DSTORM_LOCALIZATIONFILE_H
#define DSTORM_LOCALIZATIONFILE_H

#include <dStorm/Transmission.h>
#include <memory>
#include <iostream>
#include <fstream>

namespace dStorm {
    class LocalizationFile : public Transmission {
      private: 
        ost::Mutex mutex;
        std::string filename;
        int w, h, l;
        std::auto_ptr<std::ofstream> fileKeeper;
        std::ostream *file;

        void open() throw();

      public:
        LocalizationFile(const std::string &filename) throw();
        ~LocalizationFile() throw();

        AdditionalData announceStormSize(int w, int h, int l) throw();
        Result receiveLocalizations(const EngineResult&) throw();
        void deleteAllResults() throw();
        void finish() throw();

        const char *getName() throw() { return "LocalizationFile"; }
    };
}
#endif
