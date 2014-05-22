#include "engine/EngineThread.h"

#include "engine/JobInfo.h"
#include "engine/SpotFitterFactory.h"
#include "engine/SpotFitter.h"
#include "helpers/back_inserter.h"

#include "debug.h"

namespace dStorm {
namespace engine {

EngineThread::EngineThread( Config& config, Input::TraitsPtr meta_info )
: config(config),
  meta_info( meta_info ),
  position_generator(config, *meta_info),
  origMotivation( config.motivation() + meta_info->plane_count() - 1 )
{
    DEBUG("Started piston");
    if ( meta_info->plane_count() < 1 )
        throw std::runtime_error("Zero or less optical paths given for input, cannot compute.");
    if ( meta_info->fluorophore_count < 1 )
        throw std::runtime_error("Zero or less fluorophores given for input, cannot compute.");

    DEBUG("Building spot fitter with " << meta_info->fluorophore_count << " fluorophores");
    for (int fluorophore = 0; fluorophore < meta_info->fluorophore_count; ++fluorophore) {
        JobInfo info(meta_info, fluorophore, config.fit_judging_method() );
        fitter.push_back( config.spotFittingMethod().make(info) );
    }
};

void EngineThread::compute( const ImageStack& image, output::LocalizedImage* target ) 
{
    target->clear();
    position_generator.compute_positions(image);

    while (!compute_if_enough_positions(image, target)) {
        position_generator.extend_range();
        target->clear();
    }
}

bool EngineThread::compute_if_enough_positions(
    const ImageStack& image, output::LocalizedImage* target) {
    /* Motivational fitting */
    int motivation = origMotivation;
    while (motivation > 0) {
        FitPosition fit_position;
        if (!position_generator.next_position(&fit_position)) {
            DEBUG("Not enough positions saved in position generator");
            return false;
        }
        DEBUG("Trying candidate " << fit_position.transpose() << " at motivation " << motivation );
        /* Get the next spot to fit and fit it. */
        int candidate = target->size(), start = candidate;
        double best_total_residues = std::numeric_limits<double>::infinity();
        int best_found = -1;
        for (unsigned int fit_fluo = 0; fit_fluo < fitter.size(); ++fit_fluo) {
            candidate = target->size();
            int found_number = fitter[fit_fluo].fitSpot(fit_position, image, 
                spot_fitter::Implementation::iterator( boost::back_inserter( *target ) ) );
            double total_residues = 0;

            if ( found_number > 0 )
                for ( size_t i = candidate; i < target->size(); ++i )
                    total_residues = (*target)[i].fit_residues().value();
            else
                total_residues = std::numeric_limits<double>::infinity();
            DEBUG("Fitter " << fit_fluo << " found " << found_number << " with total residues " << total_residues);
            if ( total_residues < best_total_residues ) {
                for (int i = 0; i < best_found && i < found_number; ++i)
                    (*target)[start+i] = (*target)[candidate+found_number-i-1];
                best_found = found_number;
                best_total_residues = total_residues;
            }
        }
        target->resize(start+std::max<int>(0,best_found));
        for (int i = 0; i < best_found; ++i) {
            (*target)[i+start].frame_number() = image.frame_number();
        }
        if ( best_found > 0 ) {
            DEBUG("Committing " << best_found << " localizations found at position " << (*target)[start].position().transpose());
            motivation = origMotivation;
        } else {
            motivation += best_found;
            DEBUG("No localizations, decreased motivation by " << -best_found 
                  << " to " << motivation);
        }
    }

    return true;
}

}
}
