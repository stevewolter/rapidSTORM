#include <time.h>
#include <vector>
#include <sys/time.h>

namespace dummycamera {
    using std::vector;

    extern int cameraCount;
    extern double coolingRate;
    extern float surroundTemperature;

    struct Camera {
        bool cameraInitialized;
        bool coolerOn;
        float lastTemperature;
        time_t coolerSwitch;
        bool acquiring;

        bool desiredTemperatureSet;
        float desiredTemperature;

        int left, right, top, bottom;

        int detectorWidth, detectorHeight;

        int adChannel;
        int vsspeed;
        int hsspeed;
        vector< std::pair<int, vector<float> > > ad_channels_with_hsspeeds;
        vector< float > vsspeeds;

        double exp_time, acc_time, kin_time;
        int accumulations;
        /** kinetic_images may be set to -1, indicating acquisition till
         *  abort. */
        int kinetic_images;

        struct timeval start_of_acquisition;
        int ringbuffer_start, ringbuffer_end, ringbuffer_mark;

        double exp_time_per_pixel, acc_delay, kin_delay;

        int ring_buffer_size;

        int gain;

        Camera() 
        : cameraInitialized(false),
          coolerOn(false), 
          lastTemperature(surroundTemperature),
          coolerSwitch(0), acquiring(false),
          desiredTemperatureSet(false),
          desiredTemperature(surroundTemperature),
          detectorWidth(512), detectorHeight(512),
          accumulations(1),
          exp_time_per_pixel( 0.0005 / 4096 ),
          acc_delay( 0.01 ),
          kin_delay( 0.01 ),
          ring_buffer_size(30),
          gain(1)
        {
        }

        int width() { return right-left+1; }
        int height() { return bottom-top+1; }
        int acquisitionAreaSize() { return width()*height(); }

        void update_ringbuffer();

        double min_exp_time() 
            { return acquisitionAreaSize() * exp_time_per_pixel; }
        double min_acc_time() 
            { return min_exp_time() + acc_delay; }
        double min_kin_time()
            { return min_acc_time() * accumulations + kin_delay; }
    };

    extern Camera cameras[8];

    unsigned short pixelValue( int x, int y, int z );
}
