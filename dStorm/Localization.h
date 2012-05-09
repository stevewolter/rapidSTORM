#ifndef DSTORM_FIT_H
#define DSTORM_FIT_H

#include <math.h>
#include <iostream>
#include <Eigen/Core>

#include "units/nanolength.h"
#include "input/Traits.h"

#include "localization/Field.h"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/optional/optional.hpp>
#include <vector>

namespace dStorm {

class Localization  { 
  public:
    typedef localization::Field<traits::Position> Position; 
        Position position;
    typedef localization::Field<traits::ImageNumber> ImageNumber; 
        ImageNumber frame_number;
    typedef localization::Field<traits::Amplitude> Amplitude; 
        Amplitude amplitude;
    typedef localization::Field<traits::PSFWidth> PSFWidth; 
        PSFWidth psf_width;
    typedef localization::Field<traits::TwoKernelImprovement> TwoKernelImprovement; 
        TwoKernelImprovement two_kernel_improvement;
    typedef localization::Field<traits::FitResidues> FitResidues; 
        FitResidues fit_residues;
    typedef localization::Field<traits::Fluorophore> Fluorophore; 
        Fluorophore fluorophore;
    typedef localization::Field<traits::LocalBackground> LocalBackground; 
        LocalBackground local_background;
    struct Fields {
        enum Indices { Position, ImageNumber, Amplitude, CovarianceMatrix, TwoKernelImprovement, FitResidues, Fluorophore, LocalBackground, Count };
    };
    template <int Index>
    struct Traits {
        typedef typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Index> >::type::Traits type;
    };
    template <typename _Tag, int Index>
    struct Tag {
        typedef typename _Tag::template in< typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Index> >::type::Traits > in;
    };

    typedef std::vector<Localization> Children;
    boost::optional< Children > children;

  private:
    template <typename ConstOrMutLoc> struct _iterator;
  public:
    typedef _iterator<Localization> iterator;
    typedef _iterator<const Localization> const_iterator;
    inline iterator begin();
    inline iterator end();
    inline const_iterator begin() const;
    inline const_iterator end() const;

    Localization();
    Localization( const Position::Type& position, 
                    Amplitude::Type strength );
    Localization( const Localization& );
    ~Localization();
};

std::ostream&
operator<<(std::ostream &o, const Localization& loc);

}

BOOST_FUSION_ADAPT_STRUCT(
    dStorm::Localization,
    (dStorm::Localization::Position, position)
    (dStorm::Localization::ImageNumber, frame_number)
    (dStorm::Localization::Amplitude, amplitude)
    (dStorm::Localization::PSFWidth, psf_width)
    (dStorm::Localization::TwoKernelImprovement, two_kernel_improvement)
    (dStorm::Localization::FitResidues, fit_residues)
    (dStorm::Localization::Fluorophore, fluorophore)
    (dStorm::Localization::LocalBackground, local_background)
)

#endif
