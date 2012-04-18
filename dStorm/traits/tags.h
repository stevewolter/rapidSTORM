#ifndef DSTORM_TRAITS_TAGS_H
#define DSTORM_TRAITS_TAGS_H

#include <boost/static_assert.hpp>
#include <boost/mpl/vector.hpp>
#include "../Localization.h"
#include "../localization/Traits.h"

namespace dStorm {
namespace traits {

struct value_is_given_tag;
struct uncertainty_is_given_tag;

template <typename Traits, typename type>
struct in_base {
    static const bool in_traits = false, in_localization = false;
    static type get(const localization::Field<Traits>&)
        { throw std::logic_error("Method undefined for traits-only tag."); }
    static type& set(localization::Field<Traits>&)
        { throw std::logic_error("Method undefined for traits-only tag."); }
    static type get(const Traits& t)
        { throw std::logic_error("Method undefined for localization-only tag."); }
    static type& set(Traits& t)
        { throw std::logic_error("Method undefined for localization-only tag."); }
};

struct true_tag {
    static bool all_true( bool ) { return true; }
    template <int Rs, int Cs, int Fs, int MRs, int MCs>
    static Eigen::Matrix<bool,Rs,Cs,Fs,MRs,MCs> all_true( Eigen::Matrix<bool,Rs,Cs,Fs,MRs,MCs> ) 
        { return Eigen::Matrix<bool,Rs,Cs,Fs,MRs,MCs>::Constant(true); }

    template <typename Traits>
    struct in : public in_base<Traits,typename Traits::IsGivenType> {
        typedef typename Traits::IsGivenType type; 
        typedef in_base<Traits,type> base;
        static type get(const localization::Field<Traits>& t) { return base::get(t); }
        static type& set(localization::Field<Traits>& t) { return base::set(t); }
        static type get(const Traits&) { return all_true( type() ); }
        static type& set(Traits& t) { throw std::logic_error("This type is always given"); }
    };
};

struct value_tag {
    typedef value_is_given_tag is_given_tag;
    template <typename Traits>
    struct in : public in_base<Traits,typename Traits::ValueType> { 
        typedef typename Traits::ValueType type; 
        typedef in_base<Traits,type> base;
        static const bool in_localization = true;

        static type get(const localization::Field<Traits>& t) { return t.value(); }
        static type& set(localization::Field<Traits>& t) { return t.value(); }
        static type get(const Traits& t) { return base::get(t); }
        static type& set(Traits& t) { return base::set(t); }
        static std::string shorthand() { return Traits::get_shorthand(); }
    };
};

struct uncertainty_tag {
    typedef uncertainty_is_given_tag is_given_tag;
    template <typename Traits>
    struct in: public in_base<Traits,typename Traits::ValueType> { 
        typedef typename Traits::ValueType type; 
        typedef in_base<Traits,type> base;
        static const bool in_localization = true;

        static type get(const localization::Field<Traits>& t) { return t.uncertainty(); }
        static type& set(localization::Field<Traits>& t) { return t.uncertainty(); }
        static type get(const Traits& t) { return base::get(t); }
        static type& set(Traits& t) { return base::set(t); }
        static std::string shorthand() { return "sigma" + Traits::get_shorthand(); }
    };
};

struct value_is_given_tag {
    template <typename Traits>
    struct in : public in_base<Traits,typename Traits::IsGivenType> { 
        typedef typename Traits::IsGivenType type; 
        typedef in_base<Traits,type> base;
        static const bool in_traits = true;

        static type get(const localization::Field<Traits>& t) { return base::get(t); }
        static type& set(localization::Field<Traits>& t) { return base::set(t); }
        static type get(const Traits& t) { return t.is_given; }
        static type& set(Traits& t) { return t.is_given; }
        static std::string shorthand() { return Traits::get_shorthand() + "given"; }
    };
};

struct uncertainty_is_given_tag {
    template <typename Traits>
    struct in : public in_base<Traits,typename Traits::IsGivenType> { 
        typedef typename Traits::IsGivenType type; 
        typedef in_base<Traits,type> base;
        static const bool in_traits = true;

        static type get(const localization::Field<Traits>& t) { return base::get(t); }
        static type& set(localization::Field<Traits>& t) { return base::set(t); }
        static type get(const Traits& t) { return t.uncertainty_is_given; }
        static type& set(Traits& t) { return t.uncertainty_is_given; }
        static std::string shorthand() { return "sigma" + Traits::get_shorthand() + "given"; }
    };
};

struct min_tag {
    typedef true_tag is_given_tag;
    template <typename Traits>
    struct in : public in_base<Traits,typename Traits::RangeBoundType> { 
        typedef typename Traits::RangeBoundType type; 
        typedef in_base<Traits,type> base;
        static const bool in_traits = Traits::has_range;

        struct MatrixAssigner {
            Traits& t; MatrixAssigner( Traits& t ) : t(t) {}
            template <typename Type>
            MatrixAssigner& operator=( const Type& a ) const { t.range().first = a; return *this; }
            typename Traits::BoundPair::first_type& operator()( int row, int column ) const { return t.range()(row, column).first; }
            operator type&() const { return t.range().first; }
        };

        static type get(const localization::Field<Traits>& t) { return base::get(t); }
        static type& set(localization::Field<Traits>& t) { return base::set(t); }
        static type get(const Traits& t) { return t.upper_limits(); }
        static MatrixAssigner set(Traits& t) { return MatrixAssigner(t); }
        static std::string shorthand() { return Traits::get_shorthand() + "min"; }
    };
};

struct max_tag {
    typedef true_tag is_given_tag;
    template <typename Traits>
    struct in : public in_base<Traits,typename Traits::RangeBoundType> { 
        typedef typename Traits::RangeBoundType type; 
        typedef in_base<Traits,type> base;
        static const bool in_traits = Traits::has_range;

        struct MatrixAssigner {
            Traits& t; MatrixAssigner( Traits& t ) : t(t) {}
            template <typename Type>
            MatrixAssigner& operator=( const Type& a ) const { t.range().second = a; return *this; }
            typename Traits::BoundPair::first_type& operator()( int row, int column ) const { return t.range()(row, column).second; }
            operator type&() const { return t.range().second; }
        };

        static type get(const localization::Field<Traits>& t) { return base::get(t); }
        static type& set(localization::Field<Traits>& t) { return base::set(t); }
        static type get(const Traits& t) { return t.upper_limits(); }
        static MatrixAssigner set(Traits& t) { return MatrixAssigner(t); }
        static std::string shorthand() { return Traits::get_shorthand() + "max"; }
    };
};

typedef boost::mpl::vector< value_tag, uncertainty_tag, min_tag, max_tag > tags;

}
}

#endif
