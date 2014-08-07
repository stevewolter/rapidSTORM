#include "inputs/LocalizationFile.h"

#include "helpers/make_unique.hpp"
#include "localization_file/reader.h"

namespace dStorm {
namespace inputs {
namespace LocalizationFile {

std::unique_ptr<input::Link<localization::Record>> create() {
    return make_unique<dStorm::localization_file::Reader::ChainLink>();
}

}
}
}
