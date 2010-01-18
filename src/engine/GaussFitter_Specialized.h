#ifndef GAUSS_FITTER_SPECIALIZED_H
#define GAUSS_FITTER_SPECIALIZED_H

#include "GaussFitter.h"
#include <dStorm/engine/Image_impl.h>
#include <fit++/FitFunction_impl.hh>

using namespace fitpp::Exponential2D;

namespace Eigen {
    template <>
    class NumTraits<unsigned short>
        : public NumTraits<int> {};
}

namespace dStorm {
namespace engine {

template <bool Free_Sigmas, bool Residue_Analysis, bool Corr,
          int Width, int Height>
class SpecializedGaussFitter;

template <bool FS, bool Corr, int Width, int Height>
class SpecializedGaussFitter<FS, false, Corr, Width, Height>
: public GaussFitter<FS, false, Corr>::BaseTableEntry
{
  public:
    typedef For<1, (FS) ? FreeForm : FixedForm> FitGroup;
    typedef typename FitGroup::template Deriver
        <StormPixel,Width,Height,Corr> Deriver;
    typedef Width_Invariants<FS, false> Common;

  protected:
    Deriver deriver;
    typename Deriver::Position a, b, *c;
    Common& common;

  public:
    SpecializedGaussFitter(Common& common) : common(common) {}

    virtual void setSize( int width, int height ) {
        deriver.setSize( width, height );
        a.resize( width, height );
        b.resize( width, height );
    }

    virtual int fit(const Spot& spot, Localization* target,
        const Image &image, int xl, int yl );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <bool FS, bool Corr, int Width, int Height>
class SpecializedGaussFitter<FS, true, Corr, Width, Height>
: public SpecializedGaussFitter<FS, false, Corr, Width, Height>,
  public GaussFitter<FS, true, Corr>::BaseTableEntry
{
  public:
    typedef SpecializedGaussFitter<FS, false, Corr, Width, Height> Base;
    static const int DoubleWidth = (Width==Eigen::Dynamic)?Width:Width+2;
    static const int DoubleHeight = 
        (Height==Eigen::Dynamic)?Height:Height+2;

    typedef For<2, (FS) ? FreeForm : FixedForm> FitGroup;
    typedef typename FitGroup::template Deriver
        <StormPixel,DoubleWidth,DoubleHeight,Corr> Deriver;
    typedef Width_Invariants<FS, true> Common;

  protected:
    typename Deriver::Position a, b;
    Deriver deriver;
    Common& common;

  public:
    SpecializedGaussFitter(Common& common) 
        : Base(common), common(common) {}

    virtual void setSize( int width, int height ) {
        Base::setSize( width, height );
        deriver.setSize( width+2, height+2 );
        a.resize( width+2, height+2 );
        b.resize( width+2, height+2 );
    }

    virtual int fit(const Spot& spot, Localization* target,
        const Image &image, int xl, int yl );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  private:
    enum SpotState { Single, Fishy, Double };
    SpotState residue_analysis(Eigen::Vector2i* direction,
                               int xl, int yl);
    float double_fit_analysis(
        const Image& image, const Eigen::Vector2i& direction,
        int single_fit_xl, int single_fit_yl);
};

template <bool Free_Sigmas, bool Residue_Analysis, bool Corr>
template <int X, int Y>
void
GaussFitter<Free_Sigmas, Residue_Analysis, Corr>::
make_specialization_array_entry()
{
    if ( X <= 2*msx+1 && Y <= 2*msy+1 && X >= msx+1 && Y >= msy+1
            && factory[X-1][Y-1] == NULL ) 
    {
        factory[X-1][Y-1] = new TableEntryMaker<X,Y>();
    }
}

template <bool Free_Sigmas, bool Residue_Analysis, bool Corr>
template <int MinRadius, int Radius>
void
GaussFitter<Free_Sigmas, Residue_Analysis, Corr>::
fill_specialization_array()
{
    if ( Radius > MinRadius )
        fill_specialization_array
            <MinRadius,(Radius==MinRadius)?MinRadius:Radius-1>();
    make_specialization_array_entry<2*Radius+1,2*Radius+0>();
    make_specialization_array_entry<2*Radius+3,2*Radius+0>();
    make_specialization_array_entry<2*Radius+0,2*Radius+1>();
    make_specialization_array_entry<2*Radius+1,2*Radius+1>();
    make_specialization_array_entry<2*Radius+2,2*Radius+1>();
    make_specialization_array_entry<2*Radius+3,2*Radius+1>();
    make_specialization_array_entry<2*Radius+0,2*Radius+2>();
    make_specialization_array_entry<2*Radius+1,2*Radius+2>();
    make_specialization_array_entry<2*Radius+0,2*Radius+3>();
    make_specialization_array_entry<2*Radius+1,2*Radius+3>();
    //make_specialization_array_entry<Radius+1,2*Radius+1>();
    //make_specialization_array_entry<2*Radius+1,Radius+1>();
}

template <bool Free_Sigmas, bool Corr, int Width, int Height>
int 
SpecializedGaussFitter<Free_Sigmas,false, Corr, Width, Height>::
fit( const Spot &spot, Localization *target, const Image& image,
         int xl, int yl) 
{
    deriver.setData( image.ptr(), image.width, image.height );
    deriver.setUpperLeftCorner( xl, yl );

    Eigen::Matrix<double,1,1> corners =
        deriver.selectedData.template corner<1,1>(Eigen::TopLeft) +
        deriver.selectedData.template corner<1,1>(Eigen::TopRight) +
        deriver.selectedData.template corner<1,1>(Eigen::BottomRight) +
        deriver.selectedData.template corner<1,1>(Eigen::BottomLeft);
    double shift_estimate = corners.sum() / 4;
    
    StartInformation starts =
        common.set_start( spot, image, shift_estimate, &a.parameters );
     
    LOCKING("Trying to fit spot");
    std::pair<FitResult,typename Deriver::Position*>
        fitResult = 
            common.Width_Invariants<Free_Sigmas,false>::fit_function.fit(
                a, b,
                common.Width_Invariants<Free_Sigmas,false>::constants,
                deriver );
    LOCKING("Fitted spot");

    c = fitResult.second;
    bool is_good
        = c != NULL &&
          common.check_result( &c->parameters, target, starts );

    if ( is_good ) {
        target->unset_source_trace();
        return 1;
    } else
        return -1;
}

template <bool Free_Sigmas, bool Corr, int Width, int Height>
int 
SpecializedGaussFitter<Free_Sigmas,true, Corr, Width, Height>::
fit( const Spot &spot, Localization *target, const Image& image,
         int xl, int yl) 
{
    int one_fit = Base::fit( spot, target, image, xl, yl );
    if ( one_fit <= 0 )
        return one_fit;

    Eigen::Vector2i suspected_doubleSpot_direction;
    switch ( residue_analysis( &suspected_doubleSpot_direction, xl, yl ) ) 
    {
        case Single:
            target->two_kernel_improvement() = 0;
        case Double:
        case Fishy:
            target->two_kernel_improvement() = 1 - double_fit_analysis
                (image, suspected_doubleSpot_direction, xl, yl); 
    }
    return 1;
}

template <bool Free_Sigmas, bool ResAnalysis, bool Corr>
template <int X, int Y> 
struct GaussFitter<Free_Sigmas, ResAnalysis, Corr>::TableEntryMaker
: public TableEntryFactory {
    virtual BaseTableEntry* factory(Common& common) 
        { return new SpecializedGaussFitter
            <Free_Sigmas, ResAnalysis, Corr, X,Y>(common); }
};

}
}

#endif
