#include "fitter/residue_analysis/fitter.h"
#include "fit++/FitFunction_impl.hh"
#include "Fitter.h"
#include "Exponential3D_Derivatives.h"
#include "fitter/SizeSpecializing_filler.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm {
namespace fitter {

using namespace fitpp::Exponential3D;
using namespace fitpp::Exponential3D;

template <>
template <>
void SizeSpecializing< 
    gauss_3d_fitter::NaiveFitter<1,Zhuang,1> >
::create_specializations<1>();

template <>
template <>
void SizeSpecializing< 
    gauss_3d_fitter::NaiveFitter<1,Holtzer,1> >
::create_specializations<0>()
{
    this->make_specialization_array_entry<11,12>();
}

template <>
template <>
void SizeSpecializing< 
    gauss_3d_fitter::NaiveFitter<1,Holtzer,2> >
::create_specializations<0>()
{
    this->make_specialization_array_entry<11,12>();
}

template <>
template <>
void SizeSpecializing< 
    gauss_3d_fitter::NaiveFitter<1,Zhuang,1> >
::create_specializations<0>()
{
    create_specializations<1>();
#ifdef USE_SPECIALIZED_FITTERS
    this->fill_specialization_array<3,4>();
#else
    this->make_specialization_array_entry<11,12>();
#endif
}

template <>
template <>
void SizeSpecializing< 
    gauss_3d_fitter::NaiveFitter<1,Zhuang,2> >
::create_specializations<0>()
{
    this->make_specialization_array_entry<11,12>();
}

template <>
template <>
void SizeSpecializing< 
    residue_analysis::Fitter< gauss_3d_fitter::Fitter<Zhuang,1> > >
::create_specializations<1>();

template <>
template <>
void SizeSpecializing< 
    residue_analysis::Fitter< gauss_3d_fitter::Fitter<Holtzer,1> > >
::create_specializations<0>()
{
    this->make_specialization_array_entry<11,12>();
}

template <>
template <>
void SizeSpecializing< 
    residue_analysis::Fitter< gauss_3d_fitter::Fitter<Holtzer,2> > >
::create_specializations<0>()
{
    this->make_specialization_array_entry<11,12>();
}

template <>
template <>
void SizeSpecializing< 
    residue_analysis::Fitter< gauss_3d_fitter::Fitter<Zhuang,1> > >
::create_specializations<0>()
{
    create_specializations<1>();
#ifdef USE_SPECIALIZED_FITTERS
    this->fill_specialization_array<3,4>();
#else
    this->make_specialization_array_entry<11,12>();
#endif
}

template <>
template <>
void SizeSpecializing< 
    residue_analysis::Fitter< gauss_3d_fitter::Fitter<Zhuang,2> > >
::create_specializations<0>()
{
    this->make_specialization_array_entry<11,12>();
}

}
}
