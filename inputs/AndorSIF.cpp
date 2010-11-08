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
#include <dStorm/helpers/exception.h>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/algorithm/string.hpp>

#include <dStorm/input/chain/FileContext.h>

using namespace std;

namespace dStorm {
namespace input {
namespace AndorSIF {

template<typename Pixel>
Source<Pixel>::Source(boost::shared_ptr<OpenFile> file)
: Object("AndorSIF", "SIF file"),
  BaseSource(static_cast<simparm::Node&>(*this), Flags()),
  file(file),
  has_been_iterated(false)
{
}

template<typename Pixel>
Source<Pixel>::~Source()
{
}

template<typename Pixel>
typename Source<Pixel>::TraitsPtr 
Source<Pixel>::get_traits()
{
   return Source<Pixel>::TraitsPtr( file->getTraits<Pixel>().release() );
}

template<typename Pixel>
Config<Pixel>::Config() 
: simparm::Object( "AndorSIF", "Andor SIF file" )
{
}

template<typename Pixel>
Source<Pixel>* Config<Pixel>::makeSource()
{
    return new Source<Pixel>(file);
}

template<typename Pixel>
input::chain::Link::AtEnd
Config<Pixel>::context_changed( ContextRef ocontext, Link* link )
{
    Terminus::context_changed( ocontext, link );
    if ( ocontext.get() == NULL ) {
        DEBUG(this << ": Empty context provided");
        return this->notify_of_trait_change( TraitsRef() );
    }
    const chain::FileContext& context = dynamic_cast<const chain::FileContext&>( *ocontext );
    if ( file.get() == NULL || context.input_file != file->for_file() ) {
        DEBUG(this << ": Have current file " << ((file.get()) ? file->for_file() : "NONE") << " and " << context.input_file);
        if ( context.input_file == "" ) {
            if ( context.throw_errors )
                throw std::runtime_error("No input file given");
            else {
                DEBUG(this << ": No input file");
                return this->notify_of_trait_change( TraitsRef() );
            }
        }

        file.reset();
        boost::shared_ptr<chain::FileMetaInfo> rv;
        try {
            file.reset( new OpenFile( context.input_file ) );
            rv.reset( new chain::FileMetaInfo() );
            rv->traits = file->getTraits<Pixel>();
            rv->accepted_basenames.push_back( make_pair("extension_sif", ".sif") );
            DEBUG(this << ": File " << file->for_file() << " was opened");
        } catch ( ... ) {
            if ( ocontext->throw_errors )
                throw;
            else 
                rv.reset();
        }
        return this->notify_of_trait_change( rv );
    } else {
        /* No change */
        return AtEnd();
    }
}

template <typename Pixel>
class Source<Pixel>::iterator
: public boost::iterator_facade<iterator,Image,std::input_iterator_tag>
{
    mutable dStorm::Image<Pixel,2> img;
    OpenFile* src;
    mutable simparm::Node* msg;
    int count;
    mutable bool did_load;

    friend class boost::iterator_core_access;

    Image& dereference() const { 
        if ( ! did_load ) {
            DEBUG("Loading at " << count);
            std::auto_ptr<dStorm::Image<Pixel,2> > i = src->load_image<Pixel>(count, *msg);
            if ( i.get() == NULL && src->did_have_errors() ) throw dStorm::abort();
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
        return count == i.count || (src && src->did_have_errors()); 
    }

    void increment() { DEBUG("Incrementing iterator from " << count); ++count; did_load = false; img.invalidate(); }
  public:
    iterator() : src(NULL), count(0), did_load(false) {}
    iterator(Source& s, int c = 0) : src(s.file.get()), msg(&s), count(c), did_load(false)
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
    return base_iterator( iterator(*this, file->number_of_images()) );
}

template class Config<unsigned short>;
//template class Config<unsigned int>;
//template class Config<float>;

}
}
}

#endif
