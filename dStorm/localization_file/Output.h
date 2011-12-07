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
#include <boost/ptr_container/ptr_vector.hpp>
#include "field_decl.h"

namespace dStorm {
namespace localization_file {
namespace writer {

class Output : public output::OutputObject {
  private: 
    std::string filename;
    std::auto_ptr<std::ofstream> fileKeeper;
    std::ostream *file;
    input::Traits<Localization> traits;

    std::auto_ptr< Field > field;

    void open();
    template <int Field> void make_fields();
    void output( const Localization& );

    class _Config;

  public:
    typedef simparm::Structure<_Config> Config;
    typedef output::FileOutputBuilder<Output> Source;

    Output(const Config&);
    ~Output();
    Output* clone() const { 
        throw std::runtime_error(
            "LocalizationFile::clone not implemented"); }

    AdditionalData announceStormSize(const Announcement &a);
    Result receiveLocalizations(const EngineResult&);
    void propagate_signal(ProgressSignal);

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
    { 
        insert_filename_with_check( filename, present_filenames ); 
    }
};

class Output::_Config : public simparm::Object {
  protected:
    void registerNamedEntries() {
        push_back( outputFile );
        push_back( traces );
    }
  public:
    output::BasenameAdjustedFileEntry outputFile;
    simparm::BoolEntry traces;

    _Config();

    bool can_work_with(output::Capabilities cap) { 
        traces.viewable = cap.test( output::Capabilities::ClustersWithSources );
        return true; 
    }
};


}
}
}

#endif
