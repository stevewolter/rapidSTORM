#include <boost/algorithm/string.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/thread/locks.hpp>

#include "input/InputMutex.h"
#include "engine/Image.h"
#include "engine/InputTraits.h"
#include "input/MetaInfo.h"
#include "input/Choice.h"
#include "input/Forwarder.h"
#include "simparm/FileEntry.h"
#include "simparm/Group.h"

#include "inputs/FileMethod.h"
#include "signals/InputFileNameChange.h"

namespace dStorm {
namespace input {
namespace file_method {

class FileMethod
: public Forwarder
{
    friend void unit_test(TestState&);
    simparm::Group name_object;
    simparm::FileEntry input_file;
    simparm::BaseAttribute::ConnectionStore listening;

    virtual void traits_changed( TraitsRef, Link* );

    FileMethod* clone() const { return new FileMethod(*this); }
    void registerNamedEntries( simparm::NodeHandle node ) { 
        simparm::NodeHandle r = name_object.attach_ui( node );
        input_file.attach_ui(r);
        Forwarder::registerNamedEntries(r);

        listening = input_file.value.notify_on_value_change( 
            boost::bind( &FileMethod::republish_traits, this ) );
    }
    std::string name() const { return name_object.getName(); }
    std::string description() const { return name_object.getDesc(); }

    BaseSource* makeSource() { return Forwarder::makeSource(); }

    void republish_traits();

  public:
    FileMethod();
};

class FileTypeChoice 
: public Choice
{
    void insert_new_node( std::auto_ptr<Link> l, Place p ) {
        if ( p == FileReader )
            Choice::add_choice(l);
        else
            Choice::insert_new_node(l,p);
    }

  public:
    FileTypeChoice() 
        : Choice("FileType", true) {}
};

FileMethod::FileMethod()
: Forwarder(),
  name_object("FileMethod"),
  input_file("InputFile", "")
{
    Forwarder::insert_here( std::auto_ptr<Link>( new FileTypeChoice() ) );
}

void FileMethod::republish_traits()
{
    InputMutexGuard lock( global_mutex() );
    if ( current_meta_info().get() != NULL ) {
        current_meta_info()->get_signal< signals::InputFileNameChange >()( input_file() );
    }
}

class BasenameApplier {
    std::string basename;
    bool applied;

  public:
    BasenameApplier( std::string filename ) : basename(filename), applied(false) {}
    void operator()( const std::pair<std::string,std::string>& p ) {
        if ( applied ) return;
        std::string suffix = p.second;
        if ( basename.length() >= suffix.length() &&
            boost::iequals( basename.substr( basename.length() - suffix.length() ), suffix ) ) 
        {
            basename = basename.substr( 0, basename.length() - suffix.length() );
            applied = true;
        }
    }
    const std::string& get_result() const { return basename; }
};

void FileMethod::traits_changed( TraitsRef traits, Link* from )
{
    if ( traits.get() == NULL ) return update_current_meta_info( traits );
    traits->get_signal< signals::InputFileNameChange >()( input_file() );
    /* The signal might have forced a traits update that already did our
     * work for us. */
    if ( upstream_traits() != traits ) return;

    boost::shared_ptr<MetaInfo> my_traits( new MetaInfo(*traits) );
    if ( my_traits->suggested_output_basename.unformatted()() == "" ) {
        BasenameApplier a( input_file() );
        std::string new_basename = 
            std::for_each( my_traits->accepted_basenames.begin(), 
                        my_traits->accepted_basenames.end(),
                        BasenameApplier( input_file() ) ).get_result();
        my_traits->suggested_output_basename.unformatted() = new_basename;
    }
    my_traits->forbidden_filenames.insert( input_file() );
    update_current_meta_info( my_traits );
}

std::auto_ptr<Link> makeLink() {
    return std::auto_ptr<Link>( new FileMethod() );
}

}
}
}

#include "tiff/TIFF.h"
#include "test-plugin/DummyFileInput.h"
#include "dejagnu.h"
#include "simparm/text_stream/RootNode.h"

namespace dStorm {
namespace input {
namespace file_method {

void unit_test( TestState& t ) {
    boost::shared_ptr<simparm::text_stream::RootNode> io( new simparm::text_stream::RootNode() );
    FileMethod file_method;
    file_method.registerNamedEntries(io);

    file_method.insert_new_node( tiff::make_input(), FileReader );
    file_method.publish_meta_info();
    t.testrun( file_method.current_meta_info().get(), 
        "Test method publishes traits" );
    t.testrun( file_method.current_meta_info()->provides_nothing(), 
        "Test method provides nothing without an input file" );

    file_method.input_file = tiff::test_file_name;
    t.testrun( file_method.current_meta_info().get(), 
        "Test method publishes traits for TIFF file name" );
    t.testrun( ! file_method.current_meta_info()->provides_nothing(), 
        "Test method provides something for TIFF file name" );
    t.testrun( file_method.current_meta_info()->traits< dStorm::engine::ImageStack >().get() != nullptr,
        "Test method provides correct image type" );
    t.testrun( file_method.current_meta_info()->traits< dStorm::engine::ImageStack >()->plane(0).image.size[1] == 42 * camera::pixel,
        "Test method provides correct width for TIFF file name" );

    std::auto_ptr<  dStorm::input::Link > foo = dummy_file_input::make();
    file_method.insert_new_node( foo, FileReader );
    file_method.publish_meta_info();
    t.testrun( file_method.current_meta_info()->traits< dStorm::engine::ImageStack >()->plane(0).image.size[1] == 42 * camera::pixel,
        "Test method provides correct width for TIFF file name" );

    file_method.input_file = "foobar.dummy";
    t.testrun( file_method.current_meta_info().get() &&
               file_method.current_meta_info()->traits< dStorm::engine::ImageStack >().get() &&
               file_method.current_meta_info()->traits< dStorm::engine::ImageStack >()->plane(0).image.size[1] == 50 * camera::pixel,
        "Test method can change file type" );

    FileMethod copy(file_method);
    copy.registerNamedEntries(io);
    copy.publish_meta_info();
    copy.input_file = tiff::test_file_name;
    t.testrun( copy.current_meta_info().get() && 
               copy.current_meta_info()->provides<dStorm::engine::ImageStack>() &&
               copy.current_meta_info()->traits< dStorm::engine::ImageStack >()->plane(0).image.size[1] == 42 * camera::pixel,
        "Copied file method can adapt to input file" );
    file_method.input_file = "";
    t.testrun( copy.current_meta_info().get() && 
               copy.current_meta_info()->provides<dStorm::engine::ImageStack>() &&
               copy.current_meta_info()->traits< dStorm::engine::ImageStack >()->plane(0).image.size[1] == 42 * camera::pixel,
        "Copied file method is not mutated by original" );
}

}
}
}
