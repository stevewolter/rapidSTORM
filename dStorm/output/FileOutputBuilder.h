#ifndef DSTORM_FILEOUTPUTBUILDER_H
#define DSTORM_FILEOUTPUTBUILDER_H

#include "OutputBuilder.h"
#include "BasenameAdjustedFileEntry.h"

namespace dStorm {
namespace output {

template <typename BaseType>
struct OutputFileAdjuster : public BaseType {
    OutputFileAdjuster()
        { adjust_to_basename( BaseType::config.outputFile ); }
    OutputFileAdjuster(const OutputFileAdjuster<BaseType>& o)
        : BaseType(o)
        { adjust_to_basename( BaseType::config.outputFile ); }

    OutputFileAdjuster* clone() const 
        { return new OutputFileAdjuster(*this); }
};

template <typename Config, typename Output>
class FileOutputBuilder : public OutputFileAdjuster< OutputBuilder<Config,Output> > {
  public:       
    typedef OutputFileAdjuster< OutputBuilder<Config,Output> > Base;
    FileOutputBuilder() {}
    FileOutputBuilder(const FileOutputBuilder& o)
        : Base(o) {}

    FileOutputBuilder* clone() const 
        { return new FileOutputBuilder(*this); }
};

}
}

#endif
