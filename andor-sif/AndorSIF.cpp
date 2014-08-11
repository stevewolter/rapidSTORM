#include "andor-sif/AndorSIF.h"

#include <read_sif.h>
#include <errno.h>
#include <stdio.h>

#include <algorithm>
#include <cassert>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/utility.hpp>

#include "andor-sif/AndorSIF.h"
#include "andor-sif/AndorSIF_OpenFile.h"
#include "engine/Image.h"
#include "engine/Input_decl.h"
#include "helpers/make_unique.hpp"
#include "helpers/thread.h"
#include "image/Image.h"
#include "input/FileInput.h"
#include "input/Source.h"
#include "input/Source.h"
#include "simparm/FileEntry.h"
#include "simparm/Message.h"

using namespace std;

namespace dStorm {
namespace andor_sif {

/** The Source provides images from an Andor SIF file, which
    *  is the file format used by the Andor programs.
    *
    *  The Andor SIF source is parameterized by the output pixel
    *  type, which can be one of unsigned short, unsigned int and
    *  float. This is not necessarily the internal type used by
    *  the SIF file. */
class Source : public input::Source< engine::ImageStack >
{
    public:
    typedef dStorm::engine::ImageStack Image;
    typedef input::Source<Image> BaseSource;
    typedef typename BaseSource::TraitsPtr TraitsPtr;

    Source(std::auto_ptr<OpenFile> file);
    virtual ~Source();

    TraitsPtr get_traits();

    void dispatch(typename BaseSource::Messages m) { assert( ! m.any() ); }
    void set_thread_count(int num_threads) OVERRIDE {
        assert(num_threads == 1);
    }

    bool GetNext(int thread, engine::ImageStack* output) OVERRIDE;


    private:
    simparm::NodeHandle current_ui;
    void attach_ui_( simparm::NodeHandle n ) { current_ui = n; }
    std::auto_ptr<OpenFile> file;
    int count;
};

/** Config class for Source. Simple config that adds
    *  the sif extension to the input file element. */
class Config 
: public input::FileInput< Config, OpenFile, engine::ImageStack >
{
    simparm::Object name_object;
    friend class FileInput< Config, OpenFile, engine::ImageStack >;
    OpenFile* make_file( const std::string& ) const;
    void modify_meta_info( dStorm::input::MetaInfo& );
    void attach_ui( simparm::NodeHandle n ) { name_object.attach_ui(n); }
    static std::string getName() { return "AndorSIF"; }
    public:
    Config();

    Source* makeSource();

    Config* clone() const { return new Config(*this); }
};

Source::Source(std::auto_ptr<OpenFile> file)
: file(file),
  count(0)
{
}

Source::~Source()
{
}

typename Source::TraitsPtr 
Source::get_traits()
{
   return Source::TraitsPtr( file->getTraits().release() );
}

Config::Config() 
: name_object( getName() )
{
}

Source* Config::makeSource()
{
    return new Source(this->get_file());
}

OpenFile* Config::make_file( const std::string& filename ) const
{
    return new OpenFile(filename); 
}
void Config::modify_meta_info( dStorm::input::MetaInfo& i )
{
    i.accepted_basenames.push_back( make_pair("extension_sif", ".sif") );
}

bool Source::GetNext(int thread, engine::ImageStack* output) { 
    if (count >= file->number_of_images()) {
        return false;
    }

    std::auto_ptr<engine::ImageStack> i = file->load_image(count, current_ui);
    if ( i.get() != NULL )
        *output = *i;
    else
        *output = Image( engine::Image2D() );
    output->frame_number() = count++ * camera::frame;
    return true;
}

std::unique_ptr< input::Link<engine::ImageStack> > make_input() {
    return make_unique<Config>();
}

}
}
