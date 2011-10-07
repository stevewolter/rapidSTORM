#ifndef LOCPREC_SOURCEVALUEPRINTER_H
#define LOCPREC_SOURCEVALUEPRINTER_H

#include <simparm/BoostUnits.hh>
#include <dStorm/output/FileOutputBuilder.h>
#include <simparm/Entry.hh>
#include <dStorm/Engine.h>
#include <simparm/Structure.hh>
#include <dStorm/UnitEntries.h>
#include <dStorm/UnitEntries/PixelEntry.h>
#include <dStorm/UnitEntries/FrameEntry.h>
#include <dStorm/output/BasenameAdjustedFileEntry.h>
#include <fstream>
#include <memory>

namespace locprec {

using namespace boost::units;

class SourceValuePrinter : public dStorm::output::OutputObject
{
  private:
    typedef quantity<camera::length,int> Coord;
    typedef 
        Eigen::Matrix<Coord, 2, 1, Eigen::DontAlign>
        Position;
    Position from, to;
    dStorm::frame_index from_image, to_image;
    std::string filename;
    std::auto_ptr<std::ostream> output;
    class _Config;

  public:
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::FileOutputBuilder<SourceValuePrinter> Source;

    SourceValuePrinter(const Config& config);
    SourceValuePrinter(const SourceValuePrinter&);
    inline SourceValuePrinter* clone() const ;

    AdditionalData announceStormSize(const Announcement &a);
    void propagate_signal(ProgressSignal) {}

    Result receiveLocalizations(const EngineResult& e);

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
        { insert_filename_with_check( filename, present_filenames ); }
};

class SourceValuePrinter::_Config 
: public simparm::Object
{
  public:
    dStorm::output::BasenameAdjustedFileEntry outputFile;
    dStorm::IntPixelEntry left, right, top, bottom;
    dStorm::IntFrameEntry from_image, to_image;

    _Config();

    void registerNamedEntries() {
        push_back(left); 
        push_back(right);
        push_back(top);
        push_back(bottom);
        push_back(from_image);
        push_back(to_image);
        push_back(outputFile);
    }

    bool determine_output_capabilities( dStorm::output::Capabilities& )
        { return true; }
    bool can_work_with(dStorm::output::Capabilities cap) { return true; }
};

SourceValuePrinter* SourceValuePrinter::clone() const
        { return new SourceValuePrinter(*this); }

}

#endif
