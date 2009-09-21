#include <dummycamera.h>
#include <AndorCamera/System.h>
#include <cassert>
#include <string.h>
#include <simparm/IO.hh>
#include <CImgBuffer/Config.h>
#include "AndorDirect.h"
#include <CImgBuffer/Buffer.h>
#include <CImgBuffer/Slot.h>
#include <CImgBuffer/Image.h>
#include <CImgBuffer/ImageBuffer.h>
#include <iostream>
#include <iomanip>

using namespace dummycamera;
using namespace CImgBuffer;

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

        int bufSize = 300;

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

        typedef ImageBuffer< AndorDirect::CameraPixel > ImBuf;
        ImBuf buffer( source );

        assert( buffer.dimx() > 0 && buffer.dimy() > 0 );
        assert( buffer.size() == bufSize );

        for ( ImBuf::iterator i = buffer.begin(); i != buffer.end(); i++ ) {
            Claim<AndorDirect::CamImage> claim = i->claim();
            AndorDirect::CamImage& image = *claim;
            if ( image.getImageNumber() % 100 == 0 )
                std::cerr << "At image " << image.getImageNumber() << "\n";
            for ( unsigned int x = 0; x < image.width; x+= 8 )
                for ( unsigned int y = 0; y < image.height; y+= 7 )
                {
                    if (  image(x,y) != pixelValue(x,y,image.getImageNumber()+1 ) ) { 
                        std::cerr << "Pixel error for (" << x << ", " << y << ") in "
                                  << "image " << image.getImageNumber() << ": Expected "
                                  << pixelValue(x,y,image.getImageNumber()+1) << ", got "
                                  << image(x,y) << "\n";
                        std::cerr << "Upper left corner of expectation image:\n";
                        for (unsigned int y = 0; y < 10; y++) {
                            for (unsigned int x = 0; x < 10; x++)
                                std::cerr << std::setw(3) << pixelValue(x,y,image.getImageNumber()+1);
                            std::cerr << "\n";
                        }
                        std::cerr << "Upper left corner of real image:\n";
                        for (unsigned int y = 0; y < 10; y++) {
                            for (unsigned int x = 0; x < 10; x++)
                                std::cerr << std::setw(3) << image(x,y);
                            std::cerr << "\n";
                        }
                        assert( false );
                    }
                }
        }

        return 0;
    }

    STATUS("Waiting for thread termination");
    ost::Thread::joinDetached();

    return 0;
}
