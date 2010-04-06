#include "fields_impl.h"
#include "known_fields.h"
#include <boost/units/io.hpp>
#include <sstream>
#include <dStorm/output/Traits.h>

namespace dStorm {
namespace LocalizationFile {
namespace field {

const std::string type_string<float>::ident ()
    { return "floating point with . for decimals and "
      "optional scientific e-notation"; }

const std::string type_string<double>::ident()
    { return type_string<float>::ident(); }

const std::string type_string<int>::ident()
    { return "integer"; }

const std::string str(int i) {
    std::stringstream ss;
    ss << i;
    return ss.str();
}

template <typename Scalar, int Row, int Col, int Fl, int MaxR, int MaxC >
const std::string type_string< Eigen::Matrix<Scalar, Row, Col, Fl, MaxR, MaxC> >
    ::ident()
    { return "matrix with " + str(Row) + " rows and " + str(Col) + " columns "
             "with elements of type " + type_string<Scalar>::ident() +
             " given in row-major order"; }

Interface::Ptr 
Interface::parse(const XMLNode& node)
{
    if ( node.getName() != std::string("field") )
        return Ptr(NULL);

    const char* semantic_attrib = node.getAttribute("semantic");
    const char* syntax_attrib = node.getAttribute("syntax");
    if ( semantic_attrib == NULL )
        throw std::runtime_error("Field is missing "
            "semantic attribute.");
    if ( syntax_attrib == NULL )
        throw std::runtime_error("Field is missing "
            "syntax attribute.");
    std::string semantics = semantic_attrib;
    std::string syntax = syntax_attrib;

    Interface *rv;
    if      ( semantics == XCoordinate::Props::semantic )
        rv = new XCoordinate( node );
    else if ( semantics == YCoordinate::Props::semantic )
        rv = new YCoordinate( node );
    else if ( semantics == ZCoordinate::Props::semantic )
        rv = new ZCoordinate( node );
    else if ( semantics == FrameNumber::Props::semantic )
        rv = new FrameNumber( node );
    else if ( semantics == Amplitude::Props::semantic )
        rv = new Amplitude( node );
    else if ( semantics == TwoKernelImprovement::Props::semantic )
        rv = new TwoKernelImprovement( node );
    else if ( semantics == CovarianceMatrix::Props::semantic )
        rv = new CovarianceMatrix( node );
    else if ( syntax == type_string<int>::ident() )
        rv = new Unknown<int>();
    else if ( syntax == type_string<double>::ident() )
        rv = new Unknown<double>();
    else if ( syntax == type_string<float>::ident() )
        rv = new Unknown<float>();
    else
        throw std::runtime_error("Unknown syntax " + syntax + " in localization file field.");

    return Ptr(rv);
}

}
}
}
