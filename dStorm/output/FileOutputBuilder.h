#ifndef DSTORM_FILEOUTPUTBUILDER_H
#define DSTORM_FILEOUTPUTBUILDER_H

#include "OutputBuilder.h"
#include "BasenameAdjustedFileEntry.h"

namespace dStorm {
namespace output {

template <typename BaseType>
class FileOutputBuilder : public OutputBuilder<BaseType> {
  public:       
    FileOutputBuilder(
            bool failSilently = false)
        : OutputBuilder<BaseType>(failSilently)
        { adjust_to_basename( BaseType::Config::outputFile ); }
    FileOutputBuilder(const FileOutputBuilder& o)
        : OutputBuilder<BaseType>(o)
        { adjust_to_basename( BaseType::Config::outputFile ); }

    FileOutputBuilder* clone() const 
        { return new FileOutputBuilder(*this); }
};

}
}

#endif
