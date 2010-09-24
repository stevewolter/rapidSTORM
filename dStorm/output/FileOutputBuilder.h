#ifndef DSTORM_FILEOUTPUTBUILDER_H
#define DSTORM_FILEOUTPUTBUILDER_H

#include "OutputBuilder.h"
#include "BasenameAdjustedFileEntry.h"

namespace dStorm {
namespace output {

template <typename BaseType>
struct OutputFileAdjuster : public BaseType {
    OutputFileAdjuster(
            bool failSilently = false)
        : BaseType(failSilently)
        { adjust_to_basename( BaseType::Config::outputFile ); }
    OutputFileAdjuster(const OutputFileAdjuster<BaseType>& o)
        : BaseType(o)
        { adjust_to_basename( BaseType::Config::outputFile ); }

    OutputFileAdjuster* clone() const 
        { return new OutputFileAdjuster(*this); }
};

template <typename BaseType>
class FileOutputBuilder : public OutputFileAdjuster< OutputBuilder<BaseType> > {
  public:       
    typedef OutputFileAdjuster< OutputBuilder<BaseType> > Base;
    FileOutputBuilder( bool failSilently = false) 
        : Base( failSilently ) {}
    FileOutputBuilder(const FileOutputBuilder& o)
        : Base(o) {}

    FileOutputBuilder* clone() const 
        { return new FileOutputBuilder(*this); }
};

}
}

#endif
