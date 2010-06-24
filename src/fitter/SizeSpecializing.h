#ifndef DSTORM_SIZESPECIALIZING_H
#define DSTORM_SIZESPECIALIZING_H

#include "SizeSpecializing_decl.h"
#include <dStorm/engine/SpotFitter.h>
#include <boost/utility.hpp>

namespace dStorm {
namespace fitter {

template <typename BaseFitter>
class SizeSpecializing
: boost::noncopyable, public SpotFitter
{
  public:
    static const int MaxFitWidth = 17, MaxFitHeight = 17;

    static const int FitFlags = 
        (Free_Sigmas && Corr) ? fitpp::Exponential2D::FreeForm :
        (Free_Sigmas)         ? fitpp::Exponential2D::FreeForm_NoCorrelation :
                                fitpp::Exponential2D::FixedForm;
    typedef typename BaseFitter::SizeInvariants Common;

    typedef Sized<BaseFitter> BaseTableEntry;

  private:
    Common common;
    int msx, msy;

    BaseTableEntry* table[MaxFitWidth][MaxFitHeight];

    template <int X, int Y>
    void make_specialization_array_entry() ;
    template <int MinRadius, int Radius>
    void fill_specialization_array();

    template <int Level>
    void create_specializations();

  public:
    SizeSpecializing(const BaseFitter::Config&, const JobInfo&) ;
    ~SizeSpecializing(); 

    int fitSpot( const Spot& spot, const Image& image,
                 Localization* target );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
