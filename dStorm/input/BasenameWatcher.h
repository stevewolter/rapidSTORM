#ifndef DSTORM_INPUT_BASENAMEWATCHER_H
#define DSTORM_INPUT_BASENAMEWATCHER_H

#include <simparm/FileEntry.hh>
#include <simparm/Attribute.hh>
#include <cstring>

namespace dStorm {
namespace input {

/** The BasenameWatcher is a property change callback for a filename
 *  that keeps a basename attribute current. Whenever the watched
 *  file name entry changes, its list of extensions is scanned and
 *  the first extension that matches the end of the filename is
 *  selected. The selected extension is removed from the filename,
 *  and the result is assigned to the basename attribute. */
class BasenameWatcher 
: public simparm::Node::Callback
{
    typedef simparm::Attribute<std::string> StrAttr;

    simparm::FileEntry &watch;
    StrAttr &basename;

 public:
    BasenameWatcher(
        simparm::FileEntry &watch, 
        StrAttr& basename
    )
    : watch(watch), basename(basename)
    {
        receive_changes_from( watch.value );
    }

    void operator()( simparm::Node&, Cause cause, simparm::Node* )
    {
        if (cause == ValueChanged) {
            /* Extension attributes start with this name. */
            const std::string def_ext = "extension";

            std::string rv = watch();
            /* Find all string attribute childrens of watch. */
            for (simparm::Node::const_iterator
                 i=watch.begin(); i!=watch.end(); i++)
            {
                const StrAttr *a;
                std::string name = (*i)->getName();
                if ( (a = dynamic_cast<StrAttr*>(*i)) != NULL &&
                     name.substr(0,def_ext.size()) == def_ext )
                {
                    /* This is an attribute for a filename extension.
                     * Try to find this extension on the file. */
                    std::string extension = (*a)();
                    int ext_size = extension.size();
                    if ( ext_size == 0 || int(rv.size()) < ext_size ) 
                        continue;

                    std::string file_extension_part = 
                        rv.substr( rv.size() - ext_size, ext_size );

                    if ( 0 == strcasecmp( file_extension_part.c_str(),
                                          extension.c_str() ) )
                    {
                        basename = rv.substr( 0, rv.size() - ext_size );
                        break;
                    }
                }
            }
        }
    }
};

}
}

#endif
