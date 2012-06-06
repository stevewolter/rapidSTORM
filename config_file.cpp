#include <string>
#include <wx/stdpaths.h>

namespace dStorm {

std::string user_config_file() {
    wxStandardPaths::Get().GetUserDataDir()
}

std::string system_config_file();

}
