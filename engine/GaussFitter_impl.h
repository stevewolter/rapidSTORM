#ifndef DSTORM_GAUSSFITTER_IMPL_H
#define DSTORM_GAUSSFITTER_IMPL_H

#include "GaussFitter.h"
#include "GaussFitter_Width_Invariants.h"

namespace dStorm {
namespace engine {

template <bool Free_Sigmas, bool Residue_Analysis, bool Corr>
class GaussFitter
: public SpotFitter
{
  public:
    static const int MaxFitWidth = 17, MaxFitHeight = 17;

    typedef Width_Invariants<Free_Sigmas, Residue_Analysis> Common;

    struct BaseTableEntry {
        virtual ~BaseTableEntry() {}
        virtual int fit(const Spot& spot, Localization* target,
            const Image &image, int imNumber, int xl, int yl ) = 0;
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
    GaussFitter(const Config& config) 
    : common(config) ,
      msx( config.fitWidth() ), msy( config.fitHeight() )
    {
        for (int i = 0; i < MaxFitWidth-1; i++)
            for (int j = 0; j < MaxFitHeight-1; j++) {
                table[i][j] = NULL;
                factory[i][j] = NULL;
            }

        create_specializations<0>();
    }
    ~GaussFitter() {
        for (int x = 0; x < MaxFitHeight-1; x++)
          for (int y = 0; y < MaxFitHeight-1; y++) {
            if ( table[x][y] != NULL )
                delete table[x][y];
            if ( factory[x][y] != NULL )
                delete factory[x][y];
          }
    }

    int fitSpot( const Spot& spot, const Image& image,
                 int imNum, Localization* target );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
