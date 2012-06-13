#include "debug.h"

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

#include "AndorSIF.h"
#include <dStorm/input/Source.h>
#include <dStorm/engine/Image.h>
#include <stdexcept>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace dStorm {
namespace andor_sif {

Source::Source(boost::shared_ptr<OpenFile> file)
: file(file),
  has_been_iterated(false)
{
}

Source::~Source()
{
}

typename Source::TraitsPtr 
Source::get_traits( typename BaseSource::Wishes )
{
   return Source::TraitsPtr( file->getTraits().release() );
}

Config::Config() 
: name_object( getName(), "Andor SIF file" )
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

class Source::iterator
: public boost::iterator_facade<iterator,Image,std::input_iterator_tag>
{
    mutable boost::optional<Image> img;
    OpenFile* src;
    mutable simparm::NodeHandle msg;
    int count;
    mutable bool did_load;

    friend class boost::iterator_core_access;

    Image& dereference() const { 
        if ( ! img ) {
            DEBUG("Loading image " << count << " from " << this << "," << src);
            std::auto_ptr<engine::ImageStack> i = src->load_image(count, msg);
            if ( i.get() != NULL )
                img = *i;
            else
                img = Image( engine::Image2D() );
            img->frame_number() = count * camera::frame;
        }
        DEBUG("Dereferencing " << this << " to " << img->frame_number());
        return *img; 
    }
    bool equal(const iterator& i) const { 
        return count == i.count || (src && src->did_have_errors()); 
    }

    void increment() { ++count; img.reset(); }
  public:
    iterator() : src(NULL), count(0) {}
    iterator(Source& s, int c = 0) : src(s.file.get()), msg( s.current_ui ), count(c)
    {}
};

Source::base_iterator 
Source::begin() {
    return base_iterator( iterator(*this, 0) );
}
Source::base_iterator 
Source::end() {
    return base_iterator( iterator(*this, file->number_of_images()) );
}

std::auto_ptr< input::Link > make_input() {
    return std::auto_ptr< input::Link >( new Config() );
}

}
}
