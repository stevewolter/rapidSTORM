#ifndef DSTORM_EXPRESSION_EXPRESSION_H
#define DSTORM_EXPRESSION_EXPRESSION_H

#include <dStorm/Localization.h>
#include <dStorm/input/Traits.h>
#include <boost/utility.hpp>
#include <boost/optional/optional.hpp>
#include <memory>

namespace dStorm {
namespace expression {

class Boolean : public boost::noncopyable {
    struct Impl;
    std::auto_ptr<Impl> impl;
  public:
    Boolean(std::string expression);

    boost::optional<bool> insert_statics(const input::Traits<Localization>&) const;
    bool evaluate(const Localization&) const;
};

}
}

#endif
