#ifndef DSTORM_GAUSSFITTER_H
#define DSTORM_GAUSSFITTER_H

#include "GaussFitter_decl.h"
#include "GaussFitterConfig_decl.h"
#include "GaussFitter_Width_Invariants.h"
#include <dStorm/engine/SpotFitter.h>
#include <boost/utility.hpp>

namespace dStorm {
namespace engine {

template <bool Free_Sigmas, bool Residue_Analysis, bool Corr>
class GaussFitter
: boost::noncopyable, public SpotFitter
{
  public:
    static const int MaxFitWidth = 17, MaxFitHeight = 17;

    static const int FitFlags = 
        (Free_Sigmas && Corr) ? fitpp::Exponential2D::FreeForm :
        (Free_Sigmas)         ? fitpp::Exponential2D::FreeForm_NoCorrelation :
                                fitpp::Exponential2D::FixedForm;
    typedef Width_Invariants<FitFlags, Residue_Analysis> Common;

    struct BaseTableEntry {
        virtual ~BaseTableEntry() {}
        virtual int fit(const Spot& spot, Localization* target,
            const BaseImage &image, int xl, int yl ) = 0;
        virtual void setSize( int width, int height ) = 0;
    };

    struct TableEntryFactory {
        virtual ~TableEntryFactory() {}
        virtual BaseTableEntry* factory(Common& common) = 0;
    };

    template <int X, int Y> struct TableEntryMaker;

  private:
    Common common;
    int msx, msy;

    BaseTableEntry* table[MaxFitWidth][MaxFitHeight];
    TableEntryFactory* factory[MaxFitWidth][MaxFitHeight];

    std::auto_ptr<TableEntryFactory> dynamic_fitter_factory;
    std::auto_ptr<BaseTableEntry> dynamic_fitter;

    template <int X, int Y>
    void make_specialization_array_entry() ;
    template <int MinRadius, int Radius>
    void fill_specialization_array();

    template <int Level>
    void create_specializations();

  public:
    GaussFitter(const GaussFitterConfig&, const JobInfo&) ;
    ~GaussFitter(); 

    int fitSpot( const Spot& spot, const Image& image,
                 Localization* target );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
