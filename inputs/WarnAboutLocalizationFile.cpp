#include "inputs/WarnAboutLocalizationFile.h"

#include "localization_file/reader.h"
#include "helpers/make_unique.hpp"
#include "simparm/Message.h"

namespace dStorm {
namespace inputs {

class WarnAboutLocalizationFile 
: public input::FileInput< WarnAboutLocalizationFile, localization_file::Reader::File, engine::ImageStack >
{
    typedef localization_file::Reader::File File;
    simparm::Object name_object;
    simparm::NodeHandle current_ui;

    friend class input::FileInput<WarnAboutLocalizationFile,File,engine::ImageStack>;
    File* make_file( const std::string& name ) const {
        std::unique_ptr<File> f( new File( name, File::Traits() ) );
        simparm::Message m( "No file replay in localization job",
            "Reading localization table files is not supported in a localization job. "
            "Please start a replay job.",
            simparm::Message::Warning );
        m.send( current_ui );
        return NULL;
    }
    void modify_meta_info( input::MetaInfo& ) {}
    void attach_ui( simparm::NodeHandle n ) { name_object.attach_ui(n); current_ui = n; }
    static std::string getName() { return "STM"; }
public:
    WarnAboutLocalizationFile() 
        : name_object( getName(), "Localization file" )
    {
        name_object.set_user_level( simparm::Debug );
    }

    virtual input::Source<engine::ImageStack>* makeSource() {
        throw std::runtime_error("Reading from localization files is only supported by replay jobs");
    }
    virtual WarnAboutLocalizationFile* clone() const { return new WarnAboutLocalizationFile(*this); }
};

std::unique_ptr< input::Link<engine::ImageStack> > make_warn_about_localization_file() {
    return make_unique<WarnAboutLocalizationFile>();
}

}
}
