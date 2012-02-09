#include "debug.h"
#include "AffineProjection.h"
#include "ProjectionConfig.h"
#include <simparm/Object.hh>
#include <simparm/FileEntry.hh>
#include <boost/make_shared.hpp>

namespace dStorm {
namespace traits {

class AffineProjectionConfig
: public ProjectionConfig
{
    simparm::Object node;
    simparm::FileEntry micro_alignment;
    simparm::Node& getNode_() { return node; }
    boost::shared_ptr<Projection> get_projection_( const Optics<2>::Resolutions& res ) const { 
        if ( ! micro_alignment ) 
            throw std::runtime_error("An alignment matrix file must be given "
                                     "for affine alignment");
        Eigen::Matrix3f elements = Eigen::Matrix3f::Identity();
        DEBUG("Micro alignment is given as " << micro_alignment());
        std::istream& is = const_cast<simparm::FileEntry&>(micro_alignment).get_input_stream();
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                is >> elements(r,c);
        const_cast<simparm::FileEntry&>(micro_alignment).close_input_stream();
        return boost::make_shared<AffineProjection>(res[0]->in_dpm(), res[0]->in_dpm(),
            Eigen::Affine2f(elements) );
    }

    AffineProjectionConfig* clone_() const 
        { return new AffineProjectionConfig(*this); }

  public:
    AffineProjectionConfig() 
    : node("AffineProjection", "Linear alignment"),
      micro_alignment("AlignmentFile", "Plane Alignment file") 
      { node.push_back( micro_alignment ); }
    AffineProjectionConfig( const AffineProjectionConfig& o )
    : node(o.node), micro_alignment(o.micro_alignment) 
    {
        node.push_back( micro_alignment );
    }
};

std::auto_ptr<ProjectionConfig> make_affine_projection_config() {
    return std::auto_ptr<ProjectionConfig>( new AffineProjectionConfig() );
}

}
}
