#include "SharedDataSource.h"

namespace dStorm {
namespace display {

SharedDataSource::SharedDataSource( DataSource& source )
: source(&source), notify_of_closed_window_before_disconnect_(false) {}

void SharedDataSource::disconnect() {
    boost::lock_guard< boost::recursive_mutex > lock(source_mutex);
    if ( source )
        final_change = source->get_changes();
    source = NULL;
}

std::auto_ptr< Change > SharedDataSource::get_changes() {
    boost::lock_guard< boost::recursive_mutex > lock(source_mutex);
    if ( source )
        return source->get_changes();
    else 
        return final_change;
}

bool SharedDataSource::notify_of_closed_window_before_disconnect() {
    boost::lock_guard< boost::recursive_mutex > lock(source_mutex);
    return notify_of_closed_window_before_disconnect_;
}

void SharedDataSource::notice_closed_data_window() {
    boost::lock_guard< boost::recursive_mutex > lock(source_mutex);
    if ( source ) {
        source->notice_closed_data_window();
        notify_of_closed_window_before_disconnect_ = true;
    }
}

void SharedDataSource::look_up_key_values( const DataSource::PixelInfo& info, std::vector<float>& targets ) {
    boost::lock_guard< boost::recursive_mutex > lock(source_mutex);
    if ( source ) {
        source->look_up_key_values(info, targets);
    } else {
        std::fill( targets.begin(), targets.end(), std::numeric_limits<float>::quiet_NaN() );
    }
}

void SharedDataSource::notice_user_key_limits( int index, bool lower, std::string value ) {
    boost::lock_guard< boost::recursive_mutex > lock(source_mutex);
    if ( source )
        source->notice_user_key_limits( index, lower, value );
}

void SharedDataSource::notice_drawn_rectangle( int left, int right, int to, int bottom ) {
    boost::lock_guard< boost::recursive_mutex > lock(source_mutex);
    if ( source )
        source->notice_drawn_rectangle( left, right, to, bottom );
}

}
}
