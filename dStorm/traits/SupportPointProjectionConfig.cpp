#include "debug.h"
#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include "SupportPointProjection.h"
#include "ScaledProjection.h"
#include "ProjectionConfig.h"
#include <simparm/Object.hh>
#include <simparm/FileEntry.hh>
#include <dStorm/UnitEntries/PixelSize.h>
#include <simparm/Entry_Impl.hh>
#include <fstream>

namespace dStorm {
namespace traits {

using namespace boost::units;

class SupportPointProjectionConfig
: public ProjectionConfig
{
    simparm::Object node;
    simparm::FileEntry micro_alignment;
    typedef Eigen::Matrix< boost::units::quantity< nanometer_pixel_size, float >,
                           2, 1, Eigen::DontAlign > PixelSize;
    simparm::Entry<PixelSize> resolution;
    mutable boost::shared_ptr<Projection> cache;
    mutable Optics<2>::Resolutions cached_resolution;
    mutable std::string cached_file;
    mutable PixelSize cached_size;

    simparm::Node& getNode_() { return node; }
    boost::shared_ptr<Projection> get_projection_( const Optics<2>::Resolutions& res ) const { 
        bool have_cache = cache.get();
        if ( have_cache && cached_resolution == res && cached_file == micro_alignment() && cached_size == resolution() )
            /* Use cache. */ ;
        else {
            cache.reset( make_projection_(res) );
            cached_resolution = res;
            cached_file = micro_alignment();
            cached_size = resolution();
        }
        return cache;
    }

    Projection* make_projection_( const Optics<2>::Resolutions& res ) const { 
        if ( ! micro_alignment ) 
            return NULL;

        std::ifstream input ( micro_alignment().c_str(), std::ios::in );
        if ( ! input ) return NULL;
        return new SupportPointProjection(
            res[0]->in_dpm(),
            res[1]->in_dpm(),
            quantity<camera::resolution>((1E9f * si::nanometre / si::metre) / (resolution().x())),
            quantity<camera::resolution>(1E9f * si::nanometre / si::metre / (resolution().y())),
            input
        );
    }

    SupportPointProjectionConfig* clone_() const 
        { return new SupportPointProjectionConfig(*this); }

  public:
    SupportPointProjectionConfig() 
    : node("SupportPointProjection", "Support point alignment"),
      micro_alignment("AlignmentFile", "bUnwarpJ transformation"),
      resolution( "Resolution", "Transformation resolution", 
                  PixelSize::Constant(10.0f * si::nanometre / camera::pixel) )
    { 
        node.push_back( micro_alignment ); 
        node.push_back( resolution ); 
    }
    SupportPointProjectionConfig( const SupportPointProjectionConfig& o )
    : node(o.node), micro_alignment(o.micro_alignment), resolution(o.resolution),
      cache(o.cache), cached_resolution(o.cached_resolution), cached_file(o.cached_file),
      cached_size(o.cached_size) 
    {
        node.push_back( micro_alignment );
        node.push_back( resolution ); 
    }
};

std::auto_ptr<ProjectionConfig> make_support_point_projection_config() {
    return std::auto_ptr<ProjectionConfig>( new SupportPointProjectionConfig() );
}

}
}
