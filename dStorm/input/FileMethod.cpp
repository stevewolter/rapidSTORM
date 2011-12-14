#include "debug.h"

#include <boost/algorithm/string.hpp>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/chain/MetaInfo.h>

#include "FileMethod.h"
#include "InputFileNameChange.h"

namespace dStorm {
namespace input {

using namespace chain;

FileMethod::FileMethod()
: simparm::Set("FileMethod", "File"),
  chain::Forwarder(),
  simparm::Listener( simparm::Event::ValueChanged ),
  input_file("InputFile", "Input file"),
  children("FileType", "File type", true)
{
    input_file.helpID = "InputFile";
    children.helpID = "FileType";
    DEBUG("Created file method");
    push_back( input_file );
    push_back( children );
    children.set_auto_selection(false);
    children.value = NULL;
    receive_changes_from( input_file.value );
    chain::Forwarder::set_more_specialized_link_element( &children );
}

FileMethod::FileMethod(const FileMethod& o)
: simparm::Set(o),
  chain::Forwarder(),
  simparm::Listener( simparm::Event::ValueChanged ),
  input_file(o.input_file),
  children(o.children)
{
    push_back( input_file );
    push_back( children );
    receive_changes_from( input_file.value );
    chain::Forwarder::set_more_specialized_link_element( &children );

    if ( o.children.isValid() )
        children.choose( o.children.value().getNode().getName() );
}

FileMethod::~FileMethod() {}

void FileMethod::operator()( const simparm::Event& )
{
    ost::MutexLock lock( global_mutex() );
    if ( this->meta_info.get() != NULL ) {
        this->meta_info->get_signal< InputFileNameChange >()( input_file() );
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
        if ( boost::iequals( basename.substr( basename.length() - suffix.length() ), suffix ) ) {
            basename = basename.substr( 0, basename.length() - suffix.length() );
            applied = true;
        }
    }
    const std::string& get_result() const { return basename; }
};

Link::AtEnd FileMethod::traits_changed( TraitsRef traits, Link* from )
{
    Link::traits_changed( traits, from );
    if ( traits.get() == NULL ) return notify_of_trait_change( traits );
    traits->get_signal< InputFileNameChange >()( input_file() );
    /* The signal might have forced a traits update that already did our
     * work for us. */
    if ( this->more_specialized->current_traits() != traits )
        return Link::AtEnd();

    boost::shared_ptr<chain::MetaInfo> my_traits( new chain::MetaInfo(*traits) );
    if ( my_traits->suggested_output_basename.unformatted()() == "" ) {
        BasenameApplier a( input_file() );
        std::string new_basename = 
            std::for_each( my_traits->accepted_basenames.begin(), 
                        my_traits->accepted_basenames.end(),
                        BasenameApplier( input_file() ) ).get_result();
        my_traits->suggested_output_basename.unformatted() = new_basename;
    }
    my_traits->forbidden_filenames.insert( input_file() );
    return notify_of_trait_change( my_traits );
}

}
}

#include "inputs/TIFF.h"
#include "test-plugin/DummyFileInput.h"
#include "dejagnu.h"

namespace dStorm {
namespace input {

using namespace chain;

void FileMethod::unit_test( TestState& t ) {
    FileMethod file_method;

    file_method.insert_new_node( std::auto_ptr<Link>( new TIFF::ChainLink() ), FileReader );
    t.testrun( file_method.current_traits().get(), 
        "Test method publishes traits" );
    t.testrun( file_method.current_traits()->provides_nothing(), 
        "Test method provides nothing without an input file" );

    file_method.input_file = TIFF::test_file_name;
    t.testrun( file_method.current_traits().get(), 
        "Test method publishes traits for TIFF file name" );
    t.testrun( ! file_method.current_traits()->provides_nothing(), 
        "Test method provides something for TIFF file name" );
    t.testrun( file_method.current_traits()->traits< dStorm::engine::Image >(),
        "Test method provides correct image type" );
    t.testrun( file_method.current_traits()->traits< dStorm::engine::Image >()->size[1] == 42 * camera::pixel,
        "Test method provides correct width for TIFF file name" );

    file_method.insert_new_node( std::auto_ptr<Link>( new dummy_file_input::Method() ), FileReader );
    t.testrun( file_method.current_traits()->traits< dStorm::engine::Image >()->size[1] == 42 * camera::pixel,
        "Test method provides correct width for TIFF file name" );
}

}
}
