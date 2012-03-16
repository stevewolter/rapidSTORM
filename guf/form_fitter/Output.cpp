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
#include "calibrate_3d/ZTruth.h"

namespace dStorm {
namespace form_fitter {

traits::Optics::PSF max_psf_size( const input::Traits<engine::ImageStack>& traits )
{
   traits::Optics::PSF max_psf = *traits.optics(0).psf_size(0);
    for (int p = 0; p < traits.plane_count(); ++p) {
        for (std::map<int,FluorophoreTraits>::const_iterator i = traits.fluorophores.begin();
              i != traits.fluorophores.end(); ++i)
            max_psf = max_psf.max( *traits.optics(p).psf_size(i->first) );
    }
    return max_psf;
}

const int GUI::tile_rows;
const int GUI::tile_cols;

Output::Output(const Config& c)
: OutputObject("FitPSFForm", "PSF Form estimation"),
  config(c),
  engine(NULL),
  visual_select( c.visual_selection() ),
  z_truth( (config.has_z_truth()) ? config.get_z_truth().release() : NULL ),
  current_limit( 0 ),
  collection("CollectionProgress", "Spots for form estimation"),
  fit("FitProgress", "Form estimation fit progress")
{
    result_config.registerNamedEntries();
    fit.viewable = false;
    collection.userLevel = simparm::Object::Beginner;
    fit.userLevel = simparm::Object::Beginner;

    push_back( collection );
    push_back( fit );
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
    for (int plane = 0; plane < a.input_image_traits->plane_count(); ++plane)
        if ( ! boost::get<traits::No3D>( a.input_image_traits->optics(plane).depth_info().get_ptr() ) 
            && ! config.has_z_truth() && config.fit_focus_plane() )
            throw std::runtime_error("Focus planes cannot be fitted without Z ground truth");

    dStorm::traits::Optics::PSF max_psf = max_psf_size( *a.input_image_traits );
    DEBUG("Maximum PSF size is " << max_psf.transpose());
    for (int i = 0; i < 2; ++i ) {
        bounds[i] = boost::icl::interval< samplepos::Scalar >::closed(
            *a.position().range()[i].first + 3.0f*max_psf[i],
            *a.position().range()[i].second - 3.0f*max_psf[i]
        );
    } 

    if ( a.input_image_traits.get() == NULL && config.auto_disable() )
        return AdditionalData();
    else
        input.reset( new Input( config, a, max_psf ) );

    if ( z_truth.get() )
        z_truth->set_meta_info(a);
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

    EngineResult copy( engine_result );
    EngineResult::iterator end = copy.end();
    if ( z_truth.get() ) {
        end = z_truth->calibrate( copy );
        for (EngineResult::iterator i = copy.begin(); i != end; ++i)
            i->position().z() = z_truth->true_z( *i );
    }

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
    fit.viewable = true;
    DEBUG("Finally doing the fit");
    std::auto_ptr< input::Traits< engine::ImageStack > > new_traits
        ( new input::Traits< engine::ImageStack >(*input->traits) );
    fitter->fit(*new_traits, fit);

    result_config.read_traits( *new_traits );
    if ( ! this->isActive() )
        new_traits->print_psf_info( std::cerr << "Auto-guessed PSF has " ) << std::endl;
    push_back( result_config );

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
