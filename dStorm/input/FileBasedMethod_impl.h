#ifndef DSTORM_INPUT_FILEBASEDMETHOD_IMPL_H
#define DSTORM_INPUT_FILEBASEDMETHOD_IMPL_H

#include "FileBasedMethod.h"

namespace dStorm {
namespace input {

template <typename Object>
FileBasedMethod<Object>::FileBasedMethod(
    input::Config& src,
    const std::string& name,
    const std::string& desc,
    const std::string& extension_name,
    const std::string& extension 
) 
: Method<Object>(name, desc),
  simparm::Listener( simparm::Event::ValueChanged ),
  master(src),
  extension( extension_name, extension ),
  inputFile(src.inputFile)
{
    this->push_back( inputFile );
    this->push_back( src.basename );
    inputFile.push_back( this->extension );
    receive_changes_from( inputFile.value );
}

template <typename Object>
FileBasedMethod<Object>::FileBasedMethod(
    const FileBasedMethod& o,
    input::Config& src
) 
: Method<Object>(o),
  simparm::Listener( simparm::Event::ValueChanged ),
  master(src),
  extension( o.extension ),
  inputFile( src.inputFile )
{
    this->push_back( inputFile );
    this->push_back( src.basename );
    inputFile.push_back( extension );
    receive_changes_from( inputFile.value );
}

template <typename Object>
void FileBasedMethod<Object>::
    operator()( const simparm::Event& )
{
    this->output_file_basename =
        inputFile.without_extension();
}

}
}

#endif
