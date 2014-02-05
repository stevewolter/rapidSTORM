#include "inputs/LocalizationFile.h"
#include <dStorm/localization_file/reader.h>

namespace dStorm {
namespace inputs {
namespace LocalizationFile {

std::auto_ptr<input::Link> create() {
    return std::auto_ptr<input::Link>( new dStorm::localization_file::Reader::ChainLink() );
}

}
}
}
