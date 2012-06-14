#include <wx/config.h>

#include "config_file.h"
#include "installation-directory.h"
#include <boost/filesystem/operations.hpp>
#include <stdlib.h>
#include <iostream>

#include "config.h"

namespace dStorm {

boost::filesystem::path program_data_path() {
    boost::filesystem::path result(CONFIG_FILE_DIR);

    std::auto_ptr<wxConfig> config( new wxConfig(wxT(PACKAGE_TARNAME)) );
    wxString new_prefix;
    if ( config->Read(wxT("InstallationPrefix"), &new_prefix) ) {
        boost::filesystem::path old_prefix(PREFIX);
        boost::filesystem::path::iterator i = result.begin(), j = old_prefix.begin();
        while ( j != old_prefix.end() && i != result.end() ) { 
            if ( *i != *j )
                std::cerr << "Error in localizing config file: datadir is not a subdirectory of prefix" << std::endl; 
            ++i, ++j; 
        }
        boost::filesystem::path rerooted = boost::filesystem::path( std::string(new_prefix.mb_str()) );
        while ( i != result.end() ) rerooted /= *i++;
        result = rerooted;
    }
    return result;
}

boost::filesystem::path initialization_file() {
    return program_data_path() / "dstorm-config.txt";
}

}
