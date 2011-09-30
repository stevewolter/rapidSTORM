#include "SourceValuePrinter.h"
#include <boost/multi_array.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <Eigen/Array>
#include <dStorm/Image.h>

namespace locprec {

SourceValuePrinter::SourceValuePrinter(
    const Config& config
) 
: OutputObject("SourceValuePrinter", "Source image value printer")
{
    from.x() = config.left();
    from.y() = config.top();
    to.x() = config.right();
    to.y() = config.bottom();
    from_image = config.from_image();
    to_image = config.to_image();
    filename = config.outputFile();

    output.reset( new std::ofstream( filename.c_str(), std::ios_base::out | std::ios_base::trunc ) );
}

SourceValuePrinter::SourceValuePrinter(const SourceValuePrinter& o)
: dStorm::output::OutputObject(o),
  from(o.from), to(o.to), from_image(o.from_image), to_image(o.to_image),
  filename(o.filename), output( new std::ofstream(filename.c_str(), std::ios_base::out | std::ios_base::trunc ) )
{
}

dStorm::output::Output::AdditionalData
SourceValuePrinter::announceStormSize(const Announcement &a)
{
    return Output::AdditionalData().set_source_image();
}

dStorm::output::Output::Result
SourceValuePrinter::receiveLocalizations(const EngineResult& e) {
    if ( e.source.is_invalid() || e.forImage < from_image || e.forImage > to_image ) return KeepRunning;
    const dStorm::engine::Image& i = e.source;
    for (int x = std::max<int>( from.x().value(), 0 ); x < std::min( to.x(), i.width() ).value(); ++x )
        {
        for (int y = std::max<int>( from.y().value(), 0 ); y < std::min( to.y(), i.height() ).value(); ++y )
            for (int z = 0; z < i.depth_in_pixels(); ++z)
                (*output) << e.forImage.value() << " " << x << " " << y << " " << " " << z << " " << i(x,y,z) << "\n";
        (*output) << "\n";
        }

    return KeepRunning;
}

SourceValuePrinter::_Config::_Config() 
: simparm::Object("SourceValuePrinter", "Print values of pixels in source image"),
  outputFile("ToFile", "Output file", ".txt"),
  left("LeftROIBorder", "Left border"),
  right("RightROIBorder", "Right border"),
  top("TopROIBorder", "Top border"),
  bottom("BottomROIBorder", "Bottom border"),
  from_image("EarlyROIBorder", "Early border"),
  to_image("LateROIBorder", "Late border")
{
}

}
