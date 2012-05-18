#include <memory>

namespace dStorm {
namespace output { class OutputSource; }
namespace expression {

namespace source {

struct LValue;
struct Filter;

LValue* new_clone( const LValue& );

};

struct Source;

std::auto_ptr<output::OutputSource> make_output_source();

}
}
