#include "debug.h"

#include <boost/algorithm/string.hpp>
#include <dStorm/input/InputMutex.h>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/InputTraits.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/input/Choice.h>
#include <dStorm/input/Forwarder.h>
#include <simparm/FileEntry.hh>
#include <boost/ptr_container/ptr_vector.hpp>
#include <simparm/Set.hh>

#include "FileMethod.h"
#include <dStorm/signals/InputFileNameChange.h>

namespace dStorm {
namespace input {
namespace file_method {

class FileMethod
: public simparm::Set,
  public Forwarder,
  private simparm::Listener
{
    friend void unit_test(TestState&);
    simparm::FileEntry input_file;

    void operator()( const simparm::Event& );
    virtual void traits_changed( TraitsRef, Link* );

    FileMethod* clone() const { return new FileMethod(*this); }
    void registerNamedEntries( simparm::Node& node ) { 
        this->push_back( input_file );
        Forwarder::registerNamedEntries(*this);
        node.push_back( *this );
    }
    std::string name() const { return getName(); }
    std::string description() const { return getDesc(); }

    BaseSource* makeSource() { return Forwarder::makeSource(); }


  public:
    FileMethod();
    FileMethod(const FileMethod&);
    ~FileMethod();

    static void unit_test( TestState& ); 
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
        : Choice("FileType", "File type", true) {}
};

FileMethod::FileMethod()
: simparm::Set("FileMethod", "File"),
  Forwarder(),
  simparm::Listener( simparm::Event::ValueChanged ),
  input_file("InputFile", "Input file")
{
    input_file.helpID = "InputFile";
    /* TODO: children.set_help_id( "FileType" ); */
    DEBUG("Created file method");
    receive_changes_from( input_file.value );
    Forwarder::insert_here( std::auto_ptr<Link>( new FileTypeChoice() ) );
}

FileMethod::FileMethod(const FileMethod& o)
: simparm::Set(o),
  Forwarder(o),
  simparm::Listener( simparm::Event::ValueChanged ),
  input_file(o.input_file)
{
    DEBUG("Copied file method " << this << " from " << &o);
    receive_changes_from( input_file.value );
}

FileMethod::~FileMethod() {}

void FileMethod::operator()( const simparm::Event& )
{
    InputMutexGuard lock( global_mutex() );
    DEBUG( "Sending callback for filename " << input_file() << " from " << this << " to " << current_meta_info().get() );
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
    DEBUG( "Sending callback for filename " << input_file() << " from " << this << " to " << traits.get() );
    if ( traits.get() == NULL ) return update_current_meta_info( traits );
    DEBUG( "FileMethod " << this << " got traits " << traits->provides_nothing() );
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

#include "inputs/TIFF.h"
#include "test-plugin/DummyFileInput.h"
#include "dejagnu.h"

namespace dStorm {
namespace input {
namespace file_method {

void unit_test( TestState& t ) {
    FileMethod file_method;

    file_method.insert_new_node( std::auto_ptr<Link>( new TIFF::ChainLink() ), FileReader );
    file_method.publish_meta_info();
    t.testrun( file_method.current_meta_info().get(), 
        "Test method publishes traits" );
    t.testrun( file_method.current_meta_info()->provides_nothing(), 
        "Test method provides nothing without an input file" );

    file_method.input_file = TIFF::test_file_name;
    t.testrun( file_method.current_meta_info().get(), 
        "Test method publishes traits for TIFF file name" );
    t.testrun( ! file_method.current_meta_info()->provides_nothing(), 
        "Test method provides something for TIFF file name" );
    t.testrun( file_method.current_meta_info()->traits< dStorm::engine::ImageStack >(),
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
    copy.publish_meta_info();
    copy.input_file = TIFF::test_file_name;
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
