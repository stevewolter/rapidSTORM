#ifndef DSTORM_LOCALIZATION_FILE_LOCALIZATION_FIELD_H
#define DSTORM_LOCALIZATION_FILE_LOCALIZATION_FIELD_H

#include "field.h"
#include "converter.h"

namespace dStorm {
namespace localization_file {

template <int Index>
class LocalizationField : public Field {
  public:
    typedef typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Index> >::type::Traits TraitsType;
  private:
    typedef typename TraitsType::ValueType ValueType;
    typedef ValueConverter<ValueType> Converter;
    boost::shared_ptr<Converter> converter;

    friend class Field;
    static Field::Ptr try_to_parse( const TiXmlElement&, TraitsType& traits );

    static std::string ident_field( const TiXmlElement& n );
    static std::string dimen_name(int n);

    void parse_attribute_with_resolution( 
        boost::optional<ValueType>&, boost::optional<ValueType>,
        const char *);

  public:
    LocalizationField();
    LocalizationField( const TiXmlElement&, TraitsType& traits );
    ~LocalizationField();

    void set_input_unit( const std::string& unit, const Field::Traits& traits );
    void set_input_unit( const std::string& unit, const TraitsType& traits );

    void parse(std::istream& input, Localization& target);
    void write(std::ostream& output, const Localization& source);
    std::auto_ptr<TiXmlNode> makeNode( const Field::Traits& traits );

    Field* clone() const { return new LocalizationField<Index>(*this); }
};

}
}

#endif
