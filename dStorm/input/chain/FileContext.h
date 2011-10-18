#ifndef DSTORM_INPUT_FILECONTEXT_H
#define DSTORM_INPUT_FILECONTEXT_H

#include "Context.h"
#include "MetaInfo.h"

namespace dStorm {
namespace input {
namespace chain {

struct FileContext : public Context {
    FileContext( const Context& base, std::string input_file )
        : Context(base), input_file(input_file) {}
    FileContext *clone() const { return new FileContext(*this); }

    std::string input_file;
};

struct FileMetaInfo : public MetaInfo {
    virtual FileMetaInfo* clone() const { return new FileMetaInfo(*this); }

    std::list< std::pair<std::string,std::string> > accepted_basenames;
};

}
}
}

#endif
