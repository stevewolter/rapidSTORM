#include <dStorm/output/OutputSource.h>

namespace dStorm {
namespace estimate_psf_form {

class Output;
class Config;
class FittingVariant;

std::auto_ptr<output::OutputSource> make_output_source();

}
}
