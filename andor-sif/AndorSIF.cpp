#include <read_sif.h>
#include <stdexcept>
#include <cassert>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>

#include <simparm/Message.h>

#include "andor-sif/AndorSIF.h"
#include "input/Source.h"
#include "engine/Image.h"
#include <stdexcept>

#include <boost/algorithm/string.hpp>

using namespace std;

namespace dStorm {
namespace andor_sif {

Source::Source(std::auto_ptr<OpenFile> file)
: file(file),
  has_been_iterated(false),
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

std::auto_ptr< input::Link > make_input() {
    return std::auto_ptr< input::Link >( new Config() );
}

}
}
