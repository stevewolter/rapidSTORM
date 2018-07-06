#ifndef DSTORM_FORMFITTER_GUI_H
#define DSTORM_FORMFITTER_GUI_H

#include "display/DataSource.h"
#include "base/Engine.h"
#include <boost/thread/future.hpp>

#include "estimate_psf_form/Tile.h"
#include "estimate_psf_form/Input.h"

namespace dStorm {
namespace estimate_psf_form {

class GUI {
    std::unique_ptr< display::Change > make_spot_display();
    void mark_fluorophores( display::Image orig_image );
    void show_selection_window();
    dStorm::engine::Image2D::Size get_maximum_tile_size();
    static const int tile_cols = 7, tile_rows = 6;

    boost::ptr_vector<Tile> work;
    const Input& input;
    std::auto_ptr<EngineBlock> engine;
    simparm::NodeHandle ui;

  public:
    /** Constructor that eats a list of tiles and asks the user which of them should be used. */
    GUI( boost::ptr_vector<Tile>& work, const Input& input, dStorm::Engine& engine, simparm::NodeHandle ui );
    ~GUI();
    static unsigned int tiles_per_view() { return tile_cols * tile_rows; }
    boost::ptr_vector<Tile> let_user_select();
};

}
}

#endif
