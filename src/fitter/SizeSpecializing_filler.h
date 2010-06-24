#ifndef DSTORM_FITTER_SIZESPECIALIZING_H
#define DSTORM_FITTER_SIZESPECIALIZING_H

#include "SizeSpecializing.h"
#include <dStorm/engine/Config.h>
#include <dStorm/engine/JobInfo.h>

namespace dStorm {
namespace fitter {

template <typename BaseFitter> 
template <int X, int Y>
void
SizeSpecializing<BaseFitter>::
make_specialization_array_entry()
{
    if ( X <= 2*msx+1 && Y <= 2*msy+1 && X >= msx+1 && Y >= msy+1
            && factory[X-1][Y-1] == NULL ) 
    {
        factory[X-1][Y-1] = new typename BaseFitter::SpecializationInfo<X,Y>::Sized();
    }
}

template <typename BaseFitter> 
template <int MinRadius, int Radius>
void
SizeSpecializing<Free_Sigmas, Residue_Analysis, Corr>::
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
