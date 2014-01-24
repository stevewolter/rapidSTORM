#include "TIFF.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "debug.h"

#ifdef HAVE_TIFFIO_H

#include <algorithm>
#include <cassert>
#include <errno.h>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <tiffio.h>

#include <boost/units/base_units/us/inch.hpp>

#include <simparm/Entry.h>
#include <simparm/FileEntry.h>
#include <simparm/TriggerEntry.h>
#include <simparm/text_stream/RootNode.h>

#include <dStorm/engine/Image.h>
#include <dStorm/Image.h>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/image/slice.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/Source.h>
#include <dStorm/signals/InputFileNameChange.h>

#include <boost/test/unit_test.hpp>
#include "TIFFOperation.h"
#include "OpenFile.h"

using namespace std;

namespace dStorm {
namespace tiff {

using namespace dStorm::input;

/** The Source provides images from a TIFF file.
    *
    *  The TIFF source is parameterized by the output pixel
    *  type, which can be one of unsigned char, unsigned short,
    *  unsigned int and float. The loaded TIFF file must match
    *  this data type exactly; no up- or downsampling is per-
    *  formed. */
class Source : public input::Source< engine::ImageStack >
{
    typedef engine::StormPixel Pixel;
    typedef engine::ImageStack Image;
    typedef engine::Image2D Plane;
    typedef input::Source<engine::ImageStack> BaseSource;
    typedef typename BaseSource::TraitsPtr TraitsPtr;

    simparm::Entry<long> count;
    simparm::NodeHandle current_ui;
    boost::shared_ptr<simparm::Node> msg;
    int directory;

    void attach_ui_( simparm::NodeHandle n ) { current_ui = count.attach_ui( n ); }

    void set_thread_count(int num_threads) override {
        assert(num_threads == 1);
    }

    bool GetNext(int thread, engine::ImageStack* output) override;

public:
    Source(std::auto_ptr<OpenFile> file);
    virtual ~Source();

    TraitsPtr get_traits(typename BaseSource::Wishes);

    void dispatch(typename BaseSource::Messages m) { assert( ! m.any() ); }
    typename BaseSource::Capabilities capabilities() const 
        { return typename BaseSource::Capabilities(); }

private:
    std::auto_ptr<OpenFile> file;

    static void TIFF_error_handler(const char*, 
        const char *fmt, va_list ap);
    
    void throw_error();
};

/** Config class for Source. Simple config that adds
    *  the sif extension to the input file element. */
class ChainLink
: public input::FileInput<ChainLink,OpenFile>
{
public:
    ChainLink();

    ChainLink* clone() const { return new ChainLink(*this); }
    BaseSource* makeSource();
    void attach_ui( simparm::NodeHandle n ) { 
        listening[1] = config.determine_length.value.notify_on_value_change( 
            boost::bind(&input::FileInput<ChainLink,OpenFile>::reread_file_locked, this) );
        config.attach_ui(n); 
    }

private:
    Config config;
    simparm::BaseAttribute::ConnectionStore listening[2];

    friend class input::FileInput<ChainLink,OpenFile>;
    OpenFile* make_file( const std::string& ) const;
    void modify_meta_info( MetaInfo& info );
    static std::string getName() { return "TIFF"; }
};


const std::string test_file_name = "special-debug-value-rapidstorm:file.tif";

Source::Source( std::auto_ptr<OpenFile> file )
: count( "EntryCount", "Number of images in TIFF file", 0 ),
  file(file),
  directory(0)
{
}

bool Source::GetNext(int thread, engine::ImageStack* target) {
    assert(thread == 0);

    if (!file->next_image(msg)) {
        return false;
    }

    OpenFile::Image three_d = file->read_image( msg );
    *target = engine::ImageStack( directory * camera::frame );
    for (int z = 0; z < three_d.depth_in_pixels(); ++z) {
        target->push_back( three_d.slice( 2, z * camera::pixel ) );
    }
    ++directory;
    return true;
}

Config::Config()
: name_object("TIFF"),
  ignore_warnings("IgnoreLibtiffWarnings",
    "Ignore libtiff warnings", true),
  determine_length("DetermineFileLength",
    "Determine length of file", false)
{
}

ChainLink::ChainLink() 
{
}

BaseSource*
ChainLink::makeSource()
{
    return new Source( get_file() );
}

Source::TraitsPtr 
Source::get_traits(typename BaseSource::Wishes) {
    TraitsPtr rv = TraitsPtr( file->getTraits(true, count).release() );
    return rv;
}

Source::~Source() {}

static void unit_test() {
    boost::shared_ptr<simparm::text_stream::RootNode> dummy_ui( new simparm::text_stream::RootNode() );
    ChainLink l;
    l.attach_ui( dummy_ui );
    l.publish_meta_info();
    BOOST_REQUIRE( l.current_meta_info().get() );
    BOOST_CHECK( l.current_meta_info()->provides_nothing() );

    l.current_meta_info()->get_signal< signals::InputFileNameChange >()( test_file_name );
    BOOST_REQUIRE( l.current_meta_info().get() );
    BOOST_CHECK( ! l.current_meta_info()->provides_nothing() );
}
boost::unit_test::test_suite* register_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE("tiff");
    rv->add( BOOST_TEST_CASE(&unit_test) );
    return rv;
}

void ChainLink::modify_meta_info( MetaInfo& i ) {
    i.accepted_basenames.push_back( make_pair("extension_tif", ".tif") );
    i.accepted_basenames.push_back( make_pair("extension_tiff", ".tiff") );
}
OpenFile* ChainLink::make_file( const std::string& n ) const
{
    DEBUG( "Creating file structure for " << n );
    return new OpenFile( n, config, config.current_user_interface() );
}

std::auto_ptr< input::Link > make_input()
{
    return std::auto_ptr< input::Link >( new ChainLink() );
}

}
}

#endif
