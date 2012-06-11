#ifndef DSTORM_WXDISPLAY_DATASOURCECONNECTION_H
#define DSTORM_WXDISPLAY_DATASOURCECONNECTION_H

#include <memory>
#include <vector>
#include <string>
#include <dStorm/display/DataSource.h>
#include <boost/thread/recursive_mutex.hpp>
#include <simparm/wx_ui/ProtocolNode.h>

namespace dStorm {
namespace display {

class Change;
class PixelInfo;

class SharedDataSource : public DataSource {
    boost::recursive_mutex source_mutex;
    DataSource* source;
    std::auto_ptr< Change > final_change;
    bool notify_of_closed_window_before_disconnect_;
    simparm::wx_ui::ProtocolNode protocol_node;
public:
    SharedDataSource( DataSource& source, simparm::wx_ui::ProtocolNode );
    void disconnect();
    bool notify_of_closed_window_before_disconnect();

    std::auto_ptr< Change > get_changes();
    void notice_closed_data_window();
    void look_up_key_values( const DataSource::PixelInfo& info, std::vector<float>& targets );
    void notice_user_key_limits( int index, bool lower, std::string value );
    void notice_drawn_rectangle( int left, int right, int to, int bottom );
};

}
}

#endif
