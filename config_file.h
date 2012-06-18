#include <wx/config.h>
#include <boost/filesystem/path.hpp>

namespace dStorm {

std::auto_ptr<wxConfig> get_wxConfig();
boost::filesystem::path initialization_file();
boost::filesystem::path program_data_path();

}
