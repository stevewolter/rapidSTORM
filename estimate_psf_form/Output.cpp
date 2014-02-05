#include "debug.h"

#include "estimate_psf_form/Output.h"
#include "estimate_psf_form/Config.h"
#include "estimate_psf_form/Fitter.h"

#include <simparm/Node.h>

#include <dStorm/output/OutputSource.h>
#include <dStorm/output/OutputBuilder.h>
#include "core/Engine.h"
#include <dStorm/image/slice.h>
#include <dStorm/image/crop.h>
#include <dStorm/image/convert.h>
#include <dStorm/image/constructors.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/variant/get.hpp>

#include "nonlinfit/levmar/exceptions.h"
#include "calibrate_3d/ZTruth.h"

namespace dStorm {
namespace estimate_psf_form {

const int GUI::tile_rows;
const int GUI::tile_cols;

Output::Output(const Config& c)
: config(c),
  engine(NULL),
  visual_select( c.visual_selection() ),
  result_config( traits::PlaneConfig::PSFDisplay ),
  current_limit( 0 ),
  collection("CollectionProgress", "Spots for form estimation"),
  fit("FitProgress", "Form estimation fit progress")
{
    fit.hide();
    collection.set_user_level( simparm::Beginner );
    fit.set_user_level( simparm::Beginner );
}

void Output::attach_ui_( simparm::NodeHandle at ) {
    current_ui = at;

    collection.attach_ui( at );
    fit.attach_ui( at );
}

Output::~Output() {
    if ( gui_result.get_state() == boost::future_state::waiting )
        gui_result.wait();
    gui_result = boost::unique_future<Tiles>();
}

output::Output::AdditionalData
Output::announceStormSize(const Announcement& a) 
{
    engine = a.engine;

    bounds[0] = boost::icl::interval< samplepos::Scalar >::closed(
        *a.position_x().range().first + samplepos::Scalar(config.fit_window_width().x()),
        *a.position_x().range().second - samplepos::Scalar(config.fit_window_width().x())
    );
    bounds[1] = boost::icl::interval< samplepos::Scalar >::closed(
        *a.position_y().range().first + samplepos::Scalar(config.fit_window_width().y()),
        *a.position_y().range().second - samplepos::Scalar(config.fit_window_width().y())
    );

    input.reset( new Input( config, a, config.fit_window_width().cast<samplepos::Scalar>() ) );

    seen_fluorophores = std::vector<bool>( input->fluorophore_count, false );

    DEBUG( "New input traits are announced" );
    result_config.read_traits( *input->traits );
    fitter = FittingVariant::create( config, *input->traits, input->number_of_spots );
    current_limit = 0;
    return AdditionalData();
}

class DummyBind {
    boost::shared_ptr<GUI> gui;
  public:
    typedef boost::ptr_vector<Tile> result_type;
    DummyBind( boost::shared_ptr<GUI> gui ) : gui(gui) {}
    ~DummyBind() {}
    boost::ptr_vector<Tile> operator()() { 
        boost::ptr_vector<Tile> result = gui->let_user_select();  
        gui.reset();
        return result;
    }
};

void Output::receiveLocalizations(const EngineResult& engine_result)
{
    boost::lock_guard<boost::mutex> lock(mutex);
    if ( ! engine || ! input->traits.get() ) return;
    DEBUG("Started using image with engine " << engine);

    const EngineResult& copy = engine_result;
    EngineResult::const_iterator end = copy.end();

    if ( copy.begin() != end )
        current_limit += config.max_per_image();
    for (EngineResult::const_iterator i = copy.begin(); i != end; ++i) {
        DEBUG("Using localization " << i-copy.begin() << " at " << i->position().transpose() << " with type " << i->fluorophore());
        bool is_close_to_border = false;
        for (int j = 0; j < 2; ++j)
            is_close_to_border = is_close_to_border || ! ( contains( bounds[j], i->position()[j] ) );
        if ( is_close_to_border ) {
            DEBUG("Rejecting localization " << i-copy.begin() << " because it is too close to the border");
            continue;
        }
        if ( current_limit < 1 ) {
            DEBUG("Rejecting localization because we have too many");
            continue;
        } else {
            current_limit -= 1;
        }

        if ( visual_select ) {
            std::auto_ptr<Tile> tile( new Tile() );
            tile->image = *copy.source;
            tile->spot = *i;
            tile->fluorophore = i->fluorophore();
            tiles.push_back( tile );
            boost::future_state::state state = gui_result.get_state();
            if ( state == boost::future_state::uninitialized &&
                 tiles.size() >= GUI::tiles_per_view() ) 
            {
                boost::shared_ptr<GUI> gui( new GUI(tiles,*input,*engine, collection.get_user_interface_handle()) );
                boost::packaged_task< Tiles > gui_selection{ DummyBind(gui) };

                gui_result = gui_selection.get_future();

                boost::thread gui_thread( std::move(gui_selection) );
                gui_thread.detach();
            } else if ( ! gui_result.is_ready() ) {
                /* Wait a few spots more. */
            } else {
                Tiles new_tiles = gui_result.get();
                selected.transfer( selected.end(), new_tiles.begin(), new_tiles.end(), new_tiles );
                DEBUG("Got results from GUI, have now " << selected.size() << " spots");
                if ( selected.size() >= config.number_of_spots() ) {
                    try {
                        for (Tiles::const_iterator i = selected.begin(); i != selected.end(); ++i) {
                            seen_fluorophores[ i->fluorophore ] = true;
                            if ( fitter->add_image( i->image, i->spot, i->fluorophore ) ) {
                                do_the_fit();
                                break;
                            } else {
                                collection.setValue( fitter->collection_state() );
                            }
                        }
                    } catch ( const nonlinfit::levmar::SingularMatrix& o ) {
                        std::cerr << "The parameter covariance matrix in PSF estimation is singular. Please select a few more spots, this might help. If it doesn't, the programmer made a mistake in parameter checking." << std::endl;
                    } catch ( const std::runtime_error& e ) {
                        std::cerr << "PSF estimation failed: " << e.what() << std::endl;
                        engine = NULL;
                        input.reset();
                    }
                    selected.clear();
                    if ( ! engine ) return;
                } else {
                    gui_result = boost::unique_future<Tiles>();
                }
            }
        } else {
            seen_fluorophores[ i->fluorophore() ] = true;
            bool enough_images = fitter->add_image( *copy.source, *i, i->fluorophore() );
            if ( enough_images ) {
                do_the_fit(); 
                break; 
            } else {
                collection.setValue( fitter->collection_state() );
            }
        }
        DEBUG("Used localization " << i - copy.begin());
    }
    DEBUG("Finished with image");
}

void Output::do_the_fit() {
    collection.setValue(1);
    fit.setValue( 0 );
    fit.show();
    DEBUG("Finally doing the fit");
    std::auto_ptr< input::Traits< engine::ImageStack > > new_traits
        ( new input::Traits< engine::ImageStack >(*input->traits) );
    fitter->fit(*new_traits, fit);

    result_config.read_traits( *new_traits );
    if ( ! (current_ui && current_ui->isActive()) )
        new_traits->print_psf_info( std::cerr << "Auto-guessed PSF has " ) << std::endl;
    result_config.attach_ui( current_ui );

    DEBUG("Signalling restart");
    engine->change_input_traits( std::auto_ptr< input::BaseTraits >(new_traits.release()) );
    engine->restart();
    engine = NULL;
    input.reset();
}

std::auto_ptr<output::OutputSource> make_output_source()
{
    return std::auto_ptr<output::OutputSource>( new OutputBuilder< Config, Output >() );
}


}
}
