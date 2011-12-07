#define BOOST_DISABLE_ASSERTS
#include "impl.h"
#include <Eigen/Core>
#include "MotionModels.h"

namespace locprec {
namespace biplane_alignment {


Config::Config()
: Object("BiplaneAlignment", "Align biplane input"), 
            model("MotionModel", "Distortion model") 
{
    model.addChoice( new NoMotion() );
    model.addChoice( new Translation() );
    model.addChoice( new ScaledTranslation() );
    model.addChoice( new Euclidean() );
    model.addChoice( new Similarity() );
    model.addChoice( new Affine() );
}

}
}
