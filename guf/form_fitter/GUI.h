#ifndef DSTORM_FORMFITTER_GUI_H
#define DSTORM_FORMFITTER_GUI_H

#include <dStorm/display/DataSource.h>
#include <dStorm/Engine.h>
#include <boost/thread/future.hpp>

#include "Tile.h"
#include "Input.h"

namespace dStorm {
namespace form_fitter {

class GUI {
    std::auto_ptr< display::Change > make_spot_display();
    void mark_fluorophores( display::Image orig_image );
    void show_selection_window();
    dStorm::engine::Image2D::Size get_maximum_tile_size();
    static const int tile_cols = 7, tile_rows = 6;

    boost::ptr_vector<Tile> work;
    const Input& input;
    std::auto_ptr<EngineBlock> engine;

  public:
    /** Constructor that eats a list of tiles and asks the user which of them should be used. */
    GUI( boost::ptr_vector<Tile>& work, const Input& input, dStorm::Engine& engine );
    ~GUI();
    static unsigned int tiles_per_view() { return tile_cols * tile_rows; }
    boost::ptr_vector<Tile> let_user_select();
};

}
}

#endif
