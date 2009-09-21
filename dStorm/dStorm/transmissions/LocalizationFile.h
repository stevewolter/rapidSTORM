#ifndef DSTORM_LOCALIZATIONFILE_H
#define DSTORM_LOCALIZATIONFILE_H

#include <dStorm/Output.h>
#include <dStorm/FileOutputBuilder.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <simparm/FileEntry.hh>
#include <simparm/Structure.hh>

namespace dStorm {
class LocalizationFile : public simparm::Object, public Output {
    private: 
    ost::Mutex mutex;
    std::string filename;
    int w, h, l;
    std::auto_ptr<std::ofstream> fileKeeper;
    std::ostream *file;
    int localizationDepth;

    void open();

    class _Config;

    public:
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::FileOutputBuilder<LocalizationFile> Source;

    LocalizationFile(const Config&);
    ~LocalizationFile();
    LocalizationFile* clone() const { 
        throw std::runtime_error(
            "LocalizationFile::clone not implemented"); }

    AdditionalData announceStormSize(const Announcement &a);
    Result receiveLocalizations(const EngineResult&);
    void propagate_signal(ProgressSignal);
};

class LocalizationFile::_Config : public simparm::Object {
  protected:
    void registerNamedEntries() {
        push_back( outputFile );
        push_back( traces );
    }
  public:
    simparm::FileEntry outputFile;
    simparm::BoolEntry traces;

    _Config();
};


}
#endif
