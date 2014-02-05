#ifndef SMOOTHED_IMAGE_SAVE_H
#define SMOOTHED_IMAGE_SAVE_H

#include <simparm/Entry.h>
#include <simparm/FileEntry.h>
#include "output/Output.h"
#include "output/OutputBuilder.h"
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>

#include <boost/lexical_cast.hpp>
#include "display/store_image.h"
#include "display/display_normalized.hpp"
#include "image/minmax.h"
#include "image/convert.h"
#include "image/extend.h"

struct SmoothedImageSave
: public dStorm::output::Output
{
    const std::string basename;

    struct Config
    {
        simparm::FileEntry output_file_name;
        static std::string get_name() { return "SmoothedImageSave"; }
        static std::string get_description() { return get_name(); }
        static simparm::UserLevel get_user_level() { return simparm::Debug; }
        Config() : output_file_name("ToFile", "Output file basename", "") {}
        void attach_ui( simparm::NodeHandle at ) { output_file_name.attach_ui(at); }
        bool can_work_with(const dStorm::output::Capabilities&)
            {return true;}
    };

    SmoothedImageSave(const Config& config) 
        : basename( config.output_file_name() ) {}
    ~SmoothedImageSave() {}
    SmoothedImageSave* clone() const { return new SmoothedImageSave(*this); }
    AdditionalData announceStormSize(const Announcement&) { return AdditionalData(); }
    void receiveLocalizations(const EngineResult& er) {
        if ( er.smoothed.is_valid() ) {
            dStorm::display::Change c(1);
            c.do_clear = true;
            c.clear_image.background = dStorm::Pixel::Black();
            display_normalized( c, extend( er.smoothed, dStorm::Image<dStorm::engine::SmoothedPixel,3>() ) );
            dStorm::display::store_image( basename + boost::lexical_cast<std::string>(er.forImage.value()) + ".png", c );
        }
    }
};

#endif
