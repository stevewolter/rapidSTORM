#ifndef DSTORM_OUTPUT_LocalizationFileXML_H
#define DSTORM_OUTPUT_LocalizationFileXML_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <iostream>
#include <memory>

//#include <boost/units/systems/si/prefixes.hpp>
#include <boost/units/quantity.hpp>

#include <boost/optional/optional.hpp>

#include <tinyxml.h>

#include <dStorm/localization/Traits_decl.h>
#include <dStorm/output/Traits_decl.h>
#include <dStorm/Localization.h>

#include "field_decl.h"

namespace dStorm {
namespace localization_file {

template <>
struct type_string<float> { static const std::string ident(); };
template <>
struct type_string<double> { static const std::string ident(); };
template <>
struct type_string<int> { static const std::string ident(); };
template <typename UnitType, typename ValueType>
struct type_string< boost::units::quantity<UnitType,ValueType> >
    : type_string<ValueType> {};
template <typename Scalar, int Row, int Col, int Fl, int MaxR, int MaxC >
struct type_string< Eigen::Matrix<Scalar, Row, Col, Fl, MaxR, MaxC> >
    : type_string<Scalar> {};

class Field {
  public:
    typedef input::Traits<Localization> Traits;
    typedef boost::ptr_vector<Field> Fields;

    typedef std::auto_ptr<Field> Ptr;
    static Ptr parse(const TiXmlNode& node, Traits& traits);
    static std::auto_ptr<Field> construct( const Traits& traits );

    virtual ~Field() {}
    virtual void set_input_unit( const std::string&, const Field::Traits& ) {}
    virtual void write(std::ostream& output, 
                       const Localization& source) = 0;
    virtual void parse(std::istream& input, 
                       Localization& target) = 0;
    virtual std::auto_ptr<TiXmlNode> makeNode( const Traits& traits ) = 0;
    virtual boost::optional<int> get_repetition_count() { return boost::optional<int>(1); }

    virtual Field* clone() const = 0;

  private:
    template <int>
    static inline Field* create_interface( const TiXmlElement& node, Traits& traits );
};

inline Field *new_clone( const Field& i )
    { return i.clone(); }

template <int Index>
Field::Ptr create_localization_field( int row = 0, int column = 0, bool uncertainty = false );
void create_localization_fields( const Field::Traits& traits, Field::Fields& result );

}
}

#endif
