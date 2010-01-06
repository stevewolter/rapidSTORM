#ifndef DSTORM_OUTPUT_LocalizationFileXML_H
#define DSTORM_OUTPUT_LocalizationFileXML_H

#include <iostream>
#include <memory>

#include <boost/units/systems/si/prefixes.hpp>
#include <boost/units/quantity.hpp>

#include <simparm/optional.hh>

/* Berghen XML Parser */
#include <xmlParser.h>

#include <dStorm/units.h>
#include <dStorm/input/LocalizationTraits_decl.h>
#include <dStorm/output/Traits_decl.h>
#include <dStorm/Localization.h>

#include "fields_decl.h"

namespace dStorm {
namespace LocalizationFile {
namespace field {

template <>
struct type_string<float> { static const std::string ident; };
template <>
struct type_string<double> { static const std::string ident; };
template <>
struct type_string<int> { static const std::string ident; };

class Interface {
  public:
    typedef std::auto_ptr<Interface> Ptr;
    static Ptr parse(const XMLNode& node);
    virtual ~Interface() {}
    virtual void getTraits( input::Traits<Localization>& )
        const = 0;
    virtual void parse(std::istream& input, 
                       Localization& target) = 0;

    virtual Interface* clone() const = 0;
};

inline Interface *new_clone( const Interface& i )
    { return i.clone(); }

template <typename Type>
struct Unknown : public Interface {
    void getTraits( input::Traits<Localization>& )
        const {}
    void parse(std::istream& input, Localization&) 
        { Type ignore; input >> ignore; }
    Unknown<Type>* clone() const
        { return new Unknown<Type>(); }
};

template <typename Properties>
class Known : public Interface
{
  public:
    typedef typename Properties::ValueQuantity Value;
    typedef Properties Props;

  private:
    simparm::optional<Value> minimum, maximum;

  public:
    Known() {}
    Known( const Value& maxVal ) : maximum(maxVal) {}
    Known( const XMLNode& );
    Known( const output::Traits& );

    XMLNode makeNode( XMLNode& top_node );
    void getTraits( input::Traits<Localization>& ) const;
    void parse(std::istream& input, 
                       Localization& target);
    Known<Props>* clone() const
        { return new Known<Props>(*this); }
};

template <typename Properties>
class KnownWithResolution :
    public Known<Properties>
{
  public:
    typedef typename Properties::ResolutionQuantity Resolution;
    typedef typename Properties::ValueQuantity Value;

    typedef Properties Props;

  private:
    simparm::optional<Resolution> resolution;
    
  public:
    KnownWithResolution() {}
    KnownWithResolution( const Value& maxVal )
        : Known<Properties>(maxVal) {}
    KnownWithResolution( const XMLNode& );
    KnownWithResolution( const output::Traits& );

    void getTraits( input::Traits<Localization>& ) const;
    XMLNode makeNode( XMLNode& top_node );
    KnownWithResolution<Props>* clone() const
        { return new KnownWithResolution<Props>(*this); }
};

}
}
}

#endif
