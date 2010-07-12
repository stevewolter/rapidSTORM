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
    gauss_3d_fitter::NaiveFitter<1,Zhuang> >
::create_specializations<1>();

template <>
template <>
void SizeSpecializing< 
    gauss_3d_fitter::NaiveFitter<1,Holtzer> >
::create_specializations<0>()
{
#ifndef USE_SPECIALIZED_FITTERS
    this->make_specialization_array_entry<11,12>();
#endif
}

template <>
template <>
void SizeSpecializing< 
    gauss_3d_fitter::NaiveFitter<1,Zhuang> >
::create_specializations<0>()
{
    create_specializations<1>();
#ifdef USE_SPECIALIZED_FITTERS
    this->fill_specialization_array<3,4>();
#else
    this->make_specialization_array_entry<11,12>();
    this->make_specialization_array_entry<9,9>();
    this->make_specialization_array_entry<8,8>();
#endif
}

template <>
template <>
void SizeSpecializing< 
    residue_analysis::Fitter< gauss_3d_fitter::Fitter<Zhuang> > >
::create_specializations<1>();

template <>
template <>
void SizeSpecializing< 
    residue_analysis::Fitter< gauss_3d_fitter::Fitter<Holtzer> > >
::create_specializations<0>()
{
#ifndef USE_SPECIALIZED_FITTERS
    this->make_specialization_array_entry<11,12>();
#endif
}

template <>
template <>
void SizeSpecializing< 
    residue_analysis::Fitter< gauss_3d_fitter::Fitter<Zhuang> > >
::create_specializations<0>()
{
    create_specializations<1>();
#ifdef USE_SPECIALIZED_FITTERS
    this->fill_specialization_array<3,4>();
#else
    this->make_specialization_array_entry<8,8>();
    this->make_specialization_array_entry<9,9>();
    this->make_specialization_array_entry<11,12>();
#endif
}

template std::auto_ptr<Sized>
    SizeSpecializing< residue_analysis::Fitter< gauss_3d_fitter::Fitter<Holtzer> > >
    ::make_unspecialized_fitter();
template std::auto_ptr<Sized>
    SizeSpecializing< residue_analysis::Fitter< gauss_3d_fitter::Fitter<Zhuang> > >
    ::make_unspecialized_fitter();
template std::auto_ptr<Sized>
    SizeSpecializing< gauss_3d_fitter::NaiveFitter<1,Holtzer> >
    ::make_unspecialized_fitter();
template std::auto_ptr<Sized>
    SizeSpecializing< gauss_3d_fitter::NaiveFitter<1,Zhuang> >
    ::make_unspecialized_fitter();

}
}
