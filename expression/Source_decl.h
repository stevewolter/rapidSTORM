#include <memory>

namespace dStorm {
namespace output { class OutputSource; }
namespace expression {

namespace source {

class LValue;
class Filter;

LValue* new_clone( const LValue& );

};

class Source;

std::auto_ptr<output::OutputSource> make_output_source();

}
}
