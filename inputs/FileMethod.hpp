#include "inputs/FileMethod.h"

#include <boost/algorithm/string.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/thread/locks.hpp>

#include "engine/Image.h"
#include "engine/InputTraits.h"
#include "helpers/make_unique.hpp"
#include "input/Choice.hpp"
#include "input/InputMutex.h"
#include "input/MetaInfo.h"
#include "signals/InputFileNameChange.h"

namespace dStorm {
namespace inputs {

using namespace input;

template <typename Type>
FileMethod<Type>::FileMethod()
: Forwarder<Type>(make_unique<Choice<Type>>("FileType", true)),
  name_object("FileMethod"),
  input_file("InputFile", "") {}

template <typename Type>
void FileMethod<Type>::republish_traits()
{
    InputMutexGuard lock( global_mutex() );
    if ( this->current_meta_info().get() != NULL ) {
        this->current_meta_info()->template get_signal< signals::InputFileNameChange >()( input_file() );
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

template <typename Type>
void FileMethod<Type>::traits_changed( typename Link<Type>::TraitsRef traits, Link<Type>* from )
{
    if ( traits.get() == NULL ) return update_current_meta_info( traits );
    traits->template get_signal< signals::InputFileNameChange >()( input_file() );
    /* The signal might have forced a traits update that already did our
     * work for us. */
    if ( this->upstream_traits() != traits ) return;

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
    this->update_current_meta_info( my_traits );
}

template <typename Type>
void FileMethod<Type>::add_choice(std::unique_ptr<input::Link<Type>> link) {
    dynamic_cast<input::Choice<Type>&>(*this->get_more_specialized()).add_choice(
            std::move(link));
}

}
}
