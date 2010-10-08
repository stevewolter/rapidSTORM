#include "config.h"
#ifdef HAVE_LIBREADSIF

#include "debug.h"

#define CImgBuffer_SIFLOADER_CPP

#include <read_sif.h>
#include <stdexcept>
#include <cassert>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>

#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Message.hh>

#include "AndorSIF.h"
#include <dStorm/input/Source.h>
#include <dStorm/input/Source_impl.h>
#include <dStorm/ImageTraits.h>
#include <AndorCamera/Config.h>
#include <dStorm/input/BasenameWatcher.h>
#include <dStorm/input/FileBasedMethod_impl.h>
#include <dStorm/helpers/exception.h>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace dStorm {
namespace input {
namespace AndorSIF {

template<typename Pixel>
Source<Pixel>::Source(boost::shared_ptr<OpenFile> file)
: Object("AndorSIF", "SIF file"),
  BaseSource(static_cast<simparm::Node&>(*this), Flags()),
  Set("AndorSIF", "SIF file"),
  file(file),
  has_been_iterated(false)
{
   init(src);
}

template<typename Pixel>
typename Source<Pixel>::TraitsPtr 
Source<Pixel>::get_traits()
{
   return file->getTraits();
}

template<typename Pixel>
Config<Pixel>::Config() 
: simparm::Node( "AndorSIF", "Andor SIF file" )
{
}

template<typename Pixel>
Source<Pixel>* Config<Pixel>::makeSource()
{
    return new Source<Pixel>(file);
}

InputChainLink::TraitsRef 
Config<Pixel>::make_traits( const Context& context )
{
    std::string input_file = static_cast<FileContext&>( context ).input_file;
    TraitsRef rv( new Traits() );
    if ( file.get() == NULL || file->for_file() != input_file )
        file.reset( new OpenFile(input_file) );
    rv->traits = file->getTraits();
    rv->configuration_element = this;
    rv->accepted_basenames.push_back( make_pair("extension_sif", ".sif") );
    return rv;
}

void Config<Pixel>::context_changed( const Context& context )
{
    if ( file.get() == NULL || context.input_file != file->for_file() )
        this->notify_of_trait_change( make_traits(context) );
}

template <typename Pixel>
class Source<Pixel>::iterator
: public boost::iterator_facade<iterator,Image,std::input_iterator_tag>
{
    mutable dStorm::Image<Pixel,2> img;
    Source* src;
    int count;
    mutable bool did_load;

    friend class boost::iterator_core_access;

    Image& dereference() const { 
        if ( ! did_load ) {
            DEBUG("Loading at " << count);
            std::auto_ptr<dStorm::Image<Pixel,2> > i = src->load(count);
            if ( i.get() == NULL && src->had_errors ) throw dStorm::abort();
            if ( i.get() != NULL )
                img = *i;
            else
                img.invalidate();
            img.frame_number() = count * cs_units::camera::frame;
            did_load = true;
        }
        return img; 
    }
    bool equal(const iterator& i) const { 
        DEBUG( "Comparing " << count << " with " << i.count ); 
        return count == i.count || (src && src->had_errors); 
    }

    void increment() { DEBUG("Incrementing iterator from " << count); ++count; did_load = false; img.invalidate(); }
  public:
    iterator() : src(NULL), count(0), did_load(false) {}
    iterator(Source& s, int c = 0) : src(&s), count(c), did_load(false)
    {}
};

template <typename PixelType>
typename Source<PixelType>::base_iterator 
Source<PixelType>::begin() {
    return base_iterator( iterator(*this, 0) );
}
template <typename PixelType>
typename Source<PixelType>::base_iterator 
Source<PixelType>::end() {
    return base_iterator( iterator(*this, im_count) );
}

template class Config<unsigned short>;
//template class Config<unsigned int>;
//template class Config<float>;

}
}
}

#endif
