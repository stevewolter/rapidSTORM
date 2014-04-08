#ifndef DSTORM_TRAITS_AFFINEPROJECTION_H
#define DSTORM_TRAITS_AFFINEPROJECTION_H

#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace traits {

class ProjectionFactory;

boost::shared_ptr< const ProjectionFactory > test_affine_projection();

}
}

#endif
