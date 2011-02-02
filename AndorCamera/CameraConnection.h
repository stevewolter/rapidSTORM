#ifndef DSTORM_ANDORDIRECT_CAMERACONNECTION_H

#include "AndorDirect_decl.h"
#include <boost/asio/ip/tcp.hpp>
#include <dStorm/ImageTraits.h>
#include <dStorm/engine/Image.h>
#include <boost/variant/variant.hpp>

namespace dStorm {
namespace AndorCamera {

struct CameraConnection {
    typedef quantity<camera::length,int> pixel;
    CameraConnection(const std::string& hostname, int camera, const std::string& port);
    void set_traits( input::Traits<engine::Image>& );
    void send( const std::string& );
    void start_acquisition( input::Traits<engine::Image>& );
    void stop_acquisition();

    struct FetchImage { quantity<camera::time,int> frame_number; };
    struct ImageError { quantity<camera::time,int> frame_number; };
    struct EndOfAcquisition {};
    typedef boost::variant<FetchImage,ImageError,EndOfAcquisition> FrameFetch;
    FrameFetch next_frame();

    void read_data( CamImage& );
  private:
    boost::asio::ip::tcp::iostream stream;
};

}
}

#endif
