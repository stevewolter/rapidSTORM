#ifndef DSTORM_INPUT_FILEINPUT_H
#define DSTORM_INPUT_FILEINPUT_H

#include "chain/Link.h"
#include "chain/FileContext_decl.h"
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
        if ( ! current_file.is_initialized() || *current_file != filename ) {
            current_file = filename;
            reread_file();
        }
    }


  protected:
    boost::shared_ptr<FileRepresentation> get_file() const { return file; }
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
        filename_change.reset( new boost::signals2::scoped_connection
            ( info->get_signal< InputFileNameChange >().connect
                (boost::bind( &FileInput::open_file, boost::ref(*this), _1) ) ) );
        this->notify_of_trait_change( info );
    }
  public:
    FileInput() { republish_traits(); }
    FileInput( const FileInput& o ) : chain::Terminus(o), 
        current_file(o.current_file), file(o.file), error(o.error)
        { republish_traits(); }
    
    AtEnd context_changed( ContextRef, Link* ) { return AtEnd(); }
};

}
}

#endif
