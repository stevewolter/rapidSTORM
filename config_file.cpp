#include "config_file.h"
#include "installation-directory.h"
#include <boost/filesystem/operations.hpp>
#include <stdlib.h>

#include <wx/config.h>
#include "config.h"

namespace dStorm {

boost::filesystem::path initialization_file() {
    boost::filesystem::path result(CONFIG_FILE_DIR);
    result /= "dstorm-config.txt";

    std::auto_ptr<wxConfig> config( new wxConfig(wxT(PACKAGE_TARNAME)) );
    wxString prefix;
    if ( config->Read(wxT("InstallationPrefix"), &prefix) ) {
        result = boost::filesystem::path( std::string(prefix.mb_str()) ) / result;
    }
    return result;
}

}
