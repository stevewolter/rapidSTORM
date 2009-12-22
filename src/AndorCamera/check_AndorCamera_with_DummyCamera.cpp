#include <ATMCD32D.H>
#include <AndorCamera/System.h>
#include <cassert>
#include <string.h>
#include <simparm/IO.hh>
#include <dStorm/input/Config.h>
#include "AndorDirect.h"
#include <dStorm/input/Buffer.h>
#include <dStorm/input/Slot.h>
//#include <dStorm/input/Image.h>
//#include <dStorm/input/ImageBuffer.h>
#include <iostream>
#include <iomanip>
#include <ATMCD32D_simulator.h>
#include <dStorm/engine/Image_impl.h>

#include <dStorm/helpers/DisplayManager.h>
#include "wxDisplay/wxManager.h"

using namespace dStorm::input;
using namespace dStorm;
using namespace dummycamera;

void check_construction_destruction() {
    std::stringstream commands(
        "attach\n"
        "in AndorDirectConfig in CamControl in Readout in SelectImage"
        " set AimCamera = 1\n"
        "quit\n"
        "detach\n"), output;

    dummycamera::coolingRate *= 5;
    Config config;
    AndorDirect::Config andor_config (config);

    simparm::IO io(&commands, &output);
    io.push_back( andor_config );

    io.processInput();
}

int main(int argc, char *argv[]) {
    dStorm::Display::Manager::setSingleton
        ( dStorm::Display::wxManager::getSingleton() );

    ost::DebugStream::set(std::cerr);
    dummycamera::cameraCount = 2;

    cameras[0].ad_channels_with_hsspeeds.push_back(
        make_pair(16, vector<float>() ) );
    cameras[0].ad_channels_with_hsspeeds[0].second.push_back( 10 );
    cameras[0].ad_channels_with_hsspeeds.push_back( 
        make_pair(14, vector<float>() ) );
    cameras[0].ad_channels_with_hsspeeds[1].second.push_back( 1 );
    cameras[0].ad_channels_with_hsspeeds[1].second.push_back( 2 );
    cameras[0].ad_channels_with_hsspeeds[1].second.push_back( 10 );

    cameras[0].vsspeeds.push_back( 1 );
    cameras[0].vsspeeds.push_back( 2 ); 
    cameras[0].vsspeeds.push_back( 3 );
    cameras[0].vsspeeds.push_back( 3.4 );
    cameras[0].vsspeeds.push_back( 4 );

    if ( argc >= 2 && !strcmp( argv[1], "--Twiddler" ) ) {
        dummycamera::coolingRate *= 5;
        Config config;
        AndorDirect::Config andor_config (config);

        simparm::IO io(&std::cin, &std::cout);
        io.push_back( andor_config );

        io.processInput();
    } else {
        check_construction_destruction();

        int bufSize = 30;

        /* Dry-run: Attach, detach, break. */
        std::stringstream commands, output;
        commands << 
            "attach\n"
            "in AndorDirectConfig set AcquisitionLength = " << bufSize << "\n"
            "in AndorDirectConfig set AcquisitionSpeed = 0.01\n"
            "in AndorDirectConfig in CamControl in Readout in ImageReadout set RightCaptureBorder = 64\n"
            "in AndorDirectConfig in CamControl in Readout in ImageReadout set BottomCaptureBorder = 64\n";

        dummycamera::coolingRate = 50;
        Config config;
        AndorDirect::Config andor_config (config);

        simparm::IO io(&commands, &output);
        io.push_back( andor_config );

        try {
            io.processInput();
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            return 1;
        }

        std::auto_ptr<Source<AndorDirect::CamImage> >
            source ( andor_config.makeSource() );

        typedef Buffer< AndorDirect::CamImage > ImBuf;
        ImBuf buffer( source );

        assert( buffer.getTraits().dimx() > 0 
                && buffer.getTraits().dimy() > 0 );

        for ( ImBuf::iterator i = buffer.begin(); i != buffer.end(); i++ ) {
            Claim<AndorDirect::CamImage> claim = i->claim();
            AndorDirect::CamImage& image = *claim;
            for ( unsigned int x = 0; x < image.width; x+= 8 )
                for ( unsigned int y = 0; y < image.height; y+= 7 )
                {
                    assert( image(x,y) == pixelValue(x,y,claim.index()+1));
                }
        }

        return 0;
    }

    return 0;
}
