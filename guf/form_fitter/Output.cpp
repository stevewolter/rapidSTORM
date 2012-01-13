#include "debug.h"

#include "Output.h"
#include "Config.h"
#include "Fitter.h"

#include <dStorm/output/OutputSource.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/Engine.h>
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

namespace dStorm {
namespace form_fitter {

const int GUI::tile_rows;
const int GUI::tile_cols;

Output::Output(const Config& c)
: OutputObject("FitPSFForm", "PSF Form estimation"),
  config(c),
  engine(NULL),
  visual_select( c.visual_selection() )
{
    result_config.registerNamedEntries();
    push_back( result_config );
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
    if ( a.input_image_traits.get() == NULL && config.auto_disable() )
        return AdditionalData();
    else
        input.reset( new Input( config, a ) );

    seen_fluorophores = std::vector<bool>( input->fluorophore_count, false );
    for (int i = 0; i < 2; ++i ) {
        bounds[i] = boost::icl::interval< samplepos::Scalar >::closed(
            *a.position().range()[i].first + 2.0f*(*a.input_image_traits->psf_size())[i],
            *a.position().range()[i].second - 2.0f*(*a.input_image_traits->psf_size())[i]
        );
    }

    DEBUG( "New input traits are announced" );
    result_config.read_traits( *input->traits );
    fitter = FittingVariant::create( config, *input->traits, input->number_of_spots );
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

void Output::receiveLocalizations(const EngineResult& er)
{
    boost::lock_guard<boost::mutex> lock(mutex);
    if ( ! engine || ! input->traits.get() ) return;
    DEBUG("Started using image with engine " << engine);
    for (EngineResult::const_iterator i = er.begin(); i != er.end(); ++i) {
        DEBUG("Using localization " << i-er.begin() << " at " << i->position().transpose() << " with type " << i->fluorophore());
        bool is_close_to_border = false;
        for (int j = 0; j < 2; ++j)
            is_close_to_border = is_close_to_border || ! ( contains( bounds[j], i->position()[j] ) );
        if ( is_close_to_border ) {
            DEBUG("Rejecting localization " << i-er.begin() << " because it is too close to the border");
            continue;
        }
        if ( visual_select ) {
            std::auto_ptr<Tile> tile( new Tile() );
            tile->image = er.source;
            tile->spot = *i;
            tile->fluorophore = i->fluorophore();
            tiles.push_back( tile );
            boost::future_state::state state = gui_result.get_state();
            if ( state == boost::future_state::uninitialized &&
                 tiles.size() >= GUI::tiles_per_view() ) 
            {
                boost::shared_ptr<GUI> gui( new GUI(tiles,*input,*engine) );
                DummyBind bind( gui );
                boost::packaged_task< Tiles > gui_selection( bind );

                gui_result = gui_selection.get_future();

                boost::thread gui_thread( boost::move(gui_selection) );
                gui_thread.detach();
            } else if ( ! gui_result.is_ready() ) {
                /* Wait a few spots more. */
            } else {
                Tiles new_tiles = gui_result.get();
                selected.transfer( selected.end(), new_tiles.begin(), new_tiles.end(), new_tiles );
                if ( selected.size() >= config.number_of_spots() ) {
                    try {
                        for (Tiles::const_iterator i = selected.begin(); i != selected.end(); ++i) {
                            seen_fluorophores[ i->fluorophore ] = true;
                            if ( fitter->add_image( i->image, i->spot, i->fluorophore ) ) {
                                do_the_fit();
                                break;
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
            bool enough_images = fitter->add_image( er.source, *i, i->fluorophore() );
            if ( enough_images ) { do_the_fit(); break; }
        }
        DEBUG("Used localization " << i - er.begin());
    }
    DEBUG("Finished with image");
}

void Output::do_the_fit() {
    DEBUG("Finally doing the fit");
    std::vector<bool>::const_iterator unfound_fluo = std::find(seen_fluorophores.begin(), seen_fluorophores.end(), false );
    if ( false && unfound_fluo != seen_fluorophores.end() ) {
        std::cerr << "You showed me not a single instance of fluorophore " 
            << (unfound_fluo-seen_fluorophores.begin()+1)
            << ". I am not going to fit anything until you do." << std::endl;
        return;
    }
    std::auto_ptr< input::Traits< engine::Image > > new_traits;
    new_traits.reset( input->traits->clone() );
    fitter->fit(*new_traits);

    result_config.read_traits( *new_traits );
    if ( ! this->isActive() ) {
            std::cerr << "Auto-guessed PSF has " << (new_traits->psf_size()->x() * 2.35f) << " FWHM ";
            if ( boost::get< traits::Zhuang3D >(new_traits->depth_info.get_ptr()) )
                std::cerr << " and 3D widening " << boost::get< traits::Zhuang3D >( *new_traits->depth_info ).widening.transpose();
            for ( size_t i = 0; i < new_traits->fluorophores.size(); ++i )
            {
                for ( int j = 0; j < new_traits->plane_count(); ++j)
                    std::cerr << ", fluorophore " << i << " in plane " << j << 
                                 " has transmission " << new_traits->plane(j).transmission_coefficient(i);
            }
            std::cerr << std::endl;
    }

    DEBUG("Signalling restart");
    engine->change_input_traits( std::auto_ptr< input::BaseTraits >(new_traits.release()) );
    engine->restart();
    engine = NULL;
    input.reset();
}

}

namespace output {

template <>
std::auto_ptr<OutputSource> make_output_source<form_fitter::Output>()
{
    return std::auto_ptr<OutputSource>( new OutputBuilder<form_fitter::Output>() );
}


}
}
