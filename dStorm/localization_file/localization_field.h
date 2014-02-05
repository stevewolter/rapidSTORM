#ifndef DSTORM_LOCALIZATION_FILE_LOCALIZATION_FIELD_H
#define DSTORM_LOCALIZATION_FILE_LOCALIZATION_FIELD_H

#include "dStorm/localization_file/field.h"
#include "dStorm/localization_file/converter.h"

namespace dStorm {
namespace localization_file {

template <typename Tag>
class LocalizationField : public Field {
  public:
    typedef typename localization::MetaInfo<Tag> TraitsType;
  private:
    typedef typename Tag::ValueType ValueType;
    typedef ValueConverter<ValueType> Converter;
    boost::shared_ptr<Converter> converter;

    static std::string ident_field( const TiXmlElement& n );
    static std::string dimen_name(int n);

    void parse_attribute_with_resolution( 
        boost::optional<ValueType>&, boost::optional<ValueType>,
        const char *);

  public:
    LocalizationField();
    LocalizationField( const TiXmlElement&, TraitsType& traits );
    ~LocalizationField();

    static Field::Ptr try_to_parse( const TiXmlElement&, TraitsType& traits );

    void set_input_unit( const std::string& unit, const Field::Traits& traits );
    void set_input_unit( const std::string& unit, const TraitsType& traits );

    void parse(std::istream& input, Localization& target);
    void write(std::ostream& output, const Localization& source);
    std::auto_ptr<TiXmlNode> makeNode( const Field::Traits& traits );

    Field* clone() const { return new LocalizationField(*this); }
};

}
}

#endif
