#ifndef SMOOTHED_IMAGE_SAVE_H
#define SMOOTHED_IMAGE_SAVE_H

#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>

#include <boost/lexical_cast.hpp>
#include <dStorm/helpers/DisplayManager.h>
#include <dStorm/helpers/DisplayDataSource_impl.h>
#include <dStorm/image/minmax.h>
#include <dStorm/image/convert.h>

struct SmoothedImageSave
: public dStorm::output::OutputObject
{
    const std::string basename;

    struct _Config
        : public simparm::Object
        {
            simparm::FileEntry output_file_name;
            _Config() : Object("SmoothedImageSave", "SmoothedImageSave"), output_file_name("ToFile", "Output file basename") {}
            void registerNamedEntries() { push_back( output_file_name); }
            bool can_work_with(const dStorm::output::Capabilities&)
                {return true;}
        };
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::OutputBuilder<SmoothedImageSave> Source;

    SmoothedImageSave(const Config& config) 
        : OutputObject("SmoothedImageSave", "SmoothedImageSave"), basename( config.output_file_name() ) {}
    ~SmoothedImageSave() {}
    SmoothedImageSave* clone() const { return new SmoothedImageSave(*this); }
    AdditionalData announceStormSize(const Announcement&) { return AdditionalData(); }
    Result receiveLocalizations(const EngineResult& er) {
        if ( er.smoothed && er.smoothed->is_valid() ) {
            dStorm::Display::Change c(1);
            c.do_clear = true;
            c.clear_image.background = dStorm::Pixel::Black();
            c.display_normalized( *er.smoothed );
            dStorm::Display::Manager::getSingleton().store_image( basename + boost::lexical_cast<std::string>(er.forImage.value()) + ".png", c );
        }
        return KeepRunning;
    }
    void propagate_signal(ProgressSignal s) {}
};

#endif
