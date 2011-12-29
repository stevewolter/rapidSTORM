#ifndef DSTORM_EXPRESSION_SIPREFIX_H
#define DSTORM_EXPRESSION_SIPREFIX_H

#include <boost/spirit/include/qi.hpp>

namespace dStorm {
namespace expression {

struct SIPrefixTable : public boost::spirit::qi::symbols<char, double>
{
   SIPrefixTable() {
      add
         ("y", 1E-24)
         ("z", 1E-21)
         ("a", 1E-18)
         ("f", 1E-15)
         ("p", 1E-12)
         ("n", 1E-9)
         ("µ", 1E-6)
         ("m", 1E-3)
         ("c", 1E-2)
         ("d", 1E-1)
         ("da", 1E1)
         ("h", 1E2)
         ("k", 1E3) 
         ("M", 1E6) 
         ("G", 1E9) 
         ("T", 1E12) 
         ("P", 1E15) 
         ("E", 1E18) 
         ("Z", 1E21) 
         ("Y", 1E24);
   }
};

}
}

#endif
