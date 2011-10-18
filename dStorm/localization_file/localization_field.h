#ifndef DSTORM_LOCALIZATION_FILE_LOCALIZATION_FIELD_H
#define DSTORM_LOCALIZATION_FILE_LOCALIZATION_FIELD_H

#include "field.h"
#include <dStorm/traits/scalar.h>
#include "converter.h"

namespace dStorm {
namespace localization_file {

template <int Index>
class LocalizationField : public Field {
  public:
    typedef typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Index> >::type::Traits TraitsType;
  private:
    typedef traits::Scalar<TraitsType> Scalar;
    typedef ValueConverter<typename Scalar::value_type> Converter;
    Scalar scalar;
    boost::shared_ptr<Converter> converter;
    bool for_uncertainty;

    friend class Field;
    static Field::Ptr try_to_parse( const TiXmlElement&, TraitsType& traits );

    static std::string ident_field( const TiXmlElement& n );
    static std::string dimen_name(int n);

    void parse_attribute_with_resolution( 
        boost::optional<typename Scalar::value_type>&, boost::optional<typename Scalar::value_type>,
        const char *,
        const typename Scalar::resolution_type&);

  public:
    static std::string identifier(int r, int c, bool for_uncertainty);

    LocalizationField( int row = 0, int column = 0, bool uncertainty = false );
    LocalizationField( const TiXmlElement&, TraitsType& traits, int row = 0, int column = 0, bool uncertainty = false );
    ~LocalizationField();

    void set_input_unit( const std::string& unit, const Field::Traits& traits );
    void set_input_unit( const std::string& unit, const TraitsType& traits );

    void parse(std::istream& input, Localization& target);
    void write(std::ostream& output, const Localization& source);
    std::auto_ptr<TiXmlNode> makeNode( const Field::Traits& traits );

    static boost::ptr_vector<Field> make_nodes( const TraitsType& traits );

    Field* clone() const { return new LocalizationField<Index>(*this); }
};

}
}

#endif
