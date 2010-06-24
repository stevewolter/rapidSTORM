#ifndef DSTORM_SIZESPECIALIZING_H
#define DSTORM_SIZESPECIALIZING_H

#include "Sized.h"
#include "SizeSpecializing_decl.h"
#include <dStorm/engine/SpotFitter.h>
#include <dStorm/engine/JobInfo_decl.h>
#include <boost/utility.hpp>

namespace dStorm {
namespace fitter {

template <typename BaseFitter>
class SizeSpecializing
: boost::noncopyable, public engine::SpotFitter
{
  public:
    static const int MaxFitWidth = 17, MaxFitHeight = 17;

    typedef typename BaseFitter::SizeInvariants Common;

    typedef Sized BaseTableEntry;

  private:
    Common common;
    int msx, msy;

    BaseTableEntry* table[MaxFitWidth][MaxFitHeight];
    std::auto_ptr<BaseTableEntry> dynamic_fitter;

    template <int X, int Y>
    void make_specialization_array_entry() ;
    template <int MinRadius, int Radius>
    void fill_specialization_array();

    template <int Level>
    void create_specializations();

    std::auto_ptr<Sized> make_unspecialized_fitter();

  public:
    SizeSpecializing(const Common&, 
                     const engine::JobInfo&) ;
    ~SizeSpecializing(); 

    int fitSpot( const engine::Spot& spot,
                 const engine::Image& image,
                 Localization* target );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
