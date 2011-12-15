#ifndef DSTORM_INPUT_FILEINPUT_H
#define DSTORM_INPUT_FILEINPUT_H

#include <dStorm/debug.h>
#include "chain/Link.h"
#include <boost/signals2/connection.hpp>
#include <boost/exception_ptr.hpp>
#include <dStorm/input/chain/MetaInfo.h>
#include "InputFileNameChange.h"

namespace dStorm {
namespace input {

template <typename CRTP, typename FileRepresentation>
class FileInput 
: public chain::Terminus
{
    boost::optional<std::string> current_file;
    boost::shared_ptr<FileRepresentation> file;
    boost::optional< boost::exception_ptr > error;
    std::auto_ptr< boost::signals2::scoped_connection > filename_change;

    void open_file( const std::string& filename ) {
        DEBUG(this << " got callback for filename " << filename);
        if ( ! current_file.is_initialized() || *current_file != filename ) {
            current_file = filename;
            reread_file();
        }
    }


  protected:
    boost::shared_ptr<FileRepresentation> get_file() const {
        if ( file.get() )
            return file; 
        else 
            return boost::shared_ptr<FileRepresentation>( 
                static_cast<const CRTP&>(*this).make_file(*current_file) );
    }
    void reread_file() {
        file.reset();
        try {
            if ( *current_file != "" )
                file.reset( static_cast<CRTP&>(*this).make_file(*current_file) );
            else
                error = boost::copy_exception( std::runtime_error("No input file name") );
        } catch ( const std::runtime_error& e ) {
            error = boost::copy_exception(e);
        }
        republish_traits();
    }
    void republish_traits() {
        boost::shared_ptr<chain::MetaInfo> info( new chain::MetaInfo() );
        static_cast<CRTP&>(*this).modify_meta_info(*info);
        if ( file.get() )
            info->set_traits( file->getTraits().release() );
        DEBUG(this << " is unregistering from " << filename_change.get());
        filename_change.reset( new boost::signals2::scoped_connection
            ( info->get_signal< InputFileNameChange >().connect
                (boost::bind( &FileInput::open_file, boost::ref(*this), _1) ) ) );
        DEBUG(this << " registered as " << filename_change.get() << " to " << info.get());
        this->notify_of_trait_change( info );
    }
  public:
    FileInput() { republish_traits(); }
    FileInput( const FileInput& o ) : chain::Terminus(o), 
        current_file(o.current_file), file(o.file), error(o.error)
        { DEBUG("Copying file input " << &o << " to " << this); republish_traits(); }
    ~FileInput() { DEBUG("Unregistering " << filename_change.get()); }
    
    AtEnd context_changed( ContextRef, Link* ) { return AtEnd(); }
};

}
}

#endif
