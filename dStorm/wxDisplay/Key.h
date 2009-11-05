#ifndef DSTORM_DISPLAY_KEY_H
#define DSTORM_DISPLAY_KEY_H

#include <wx/wx.h>
#include <memory>
#include <vector>
#include "dStorm/helpers/DisplayDataSource.h"

namespace dStorm {
namespace Display {

class Key : public wxWindow {
    wxSize current_size;
    std::auto_ptr<wxBitmap> buffer;
    int num_keys, max_text_width, max_text_height;

    std::vector<Color> colors;
    std::vector<float> values;

    const int unlabelled_width, labelled_width;
    const int top_border, bottom_border, left_border;

    int key_distance, label_distance, line_height;
    int text_height;

    wxPen background_pen;
    wxBrush background_brush;

    void compute_key_size();

    /* Center rect vertically with respect to other rect. Does what
     * wxRect::CentreIn should do, but doesn't in 2.8. */
    void center_rect_vertically( wxRect& centre, const wxRect& in );

    void draw_key( int index, wxDC &dc );

  public:
    Key( wxWindow* parent, wxSize size, int num_keys ) ;
    ~Key();

    void draw_keys( const data_cpp::Vector<KeyChange>& kcs );

    void OnPaint( wxPaintEvent& event );
    void OnResize( wxSizeEvent& );

    void resize( int new_number_of_keys );
    DECLARE_EVENT_TABLE();
};

}
}

#endif
