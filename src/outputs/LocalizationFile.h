#ifndef DSTORM_LOCALIZATIONFILE_H
#define DSTORM_LOCALIZATIONFILE_H

#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/output/Output.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <simparm/FileEntry.hh>
#include <simparm/Structure.hh>

namespace dStorm {
namespace output {
class LocalizationFile : public OutputObject {
  private: 
    ost::Mutex mutex;
    std::string filename;
    Traits traits;
    std::auto_ptr<std::ofstream> fileKeeper;
    std::ostream *file;
    int localizationDepth;

    Eigen::IOFormat format;

    void open();
    void printFit(const Localization &f, int localizationDepth);

    class _Config;

  public:
    typedef simparm::Structure<_Config> Config;
    typedef FileOutputBuilder<LocalizationFile> Source;

    LocalizationFile(const Config&);
    ~LocalizationFile();
    LocalizationFile* clone() const { 
        throw std::runtime_error(
            "LocalizationFile::clone not implemented"); }

    AdditionalData announceStormSize(const Announcement &a);
    Result receiveLocalizations(const EngineResult&);
    void propagate_signal(ProgressSignal);

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
        { insert_filename_with_check( filename, present_filenames ); }
};

class LocalizationFile::_Config : public simparm::Object {
  protected:
    void registerNamedEntries() {
        push_back( outputFile );
        push_back( traces );
    }
  public:
    BasenameAdjustedFileEntry outputFile;
    simparm::BoolEntry traces;

    _Config();

    bool can_work_with(Capabilities cap) { 
        traces.viewable = cap.test( Capabilities::ClustersWithSources );
        return true; 
    }
};


}
}
#endif
