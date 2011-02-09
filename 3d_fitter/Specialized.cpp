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
    this->make_specialization_array_entry<9,9>();
    this->make_specialization_array_entry<8,8>();
#endif
}

template <>
template <>
void SizeSpecializing< 
    gauss_3d_fitter::NaiveFitter<1,Zhuang,2> >
::create_specializations<0>()
{
    this->make_specialization_array_entry<11,12>();
    this->make_specialization_array_entry<9,9>();
    this->make_specialization_array_entry<8,8>();
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
    this->make_specialization_array_entry<8,8>();
    this->make_specialization_array_entry<9,9>();
    this->make_specialization_array_entry<11,12>();
#endif
}

template <>
template <>
void SizeSpecializing< 
    residue_analysis::Fitter< gauss_3d_fitter::Fitter<Zhuang,2> > >
::create_specializations<0>()
{
    this->make_specialization_array_entry<8,8>();
    this->make_specialization_array_entry<9,9>();
    this->make_specialization_array_entry<11,12>();
}

#define INSTANTIATE_NAIVE_AND_RESANALYZE(f,d) \
template std::auto_ptr<Sized> \
    SizeSpecializing< residue_analysis::Fitter< gauss_3d_fitter::Fitter<f,d> > > \
    ::make_unspecialized_fitter(); \
template std::auto_ptr<Sized> \
    SizeSpecializing< gauss_3d_fitter::NaiveFitter<1,f,d> > \
    ::make_unspecialized_fitter();

INSTANTIATE_NAIVE_AND_RESANALYZE(Holtzer,1)
INSTANTIATE_NAIVE_AND_RESANALYZE(Holtzer,2)
INSTANTIATE_NAIVE_AND_RESANALYZE(Zhuang,1)
INSTANTIATE_NAIVE_AND_RESANALYZE(Zhuang,2)

}
}
