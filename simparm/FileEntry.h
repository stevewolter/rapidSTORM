#ifndef SIMPARM_CONFIG_ENTRY_FILE
#define SIMPARM_CONFIG_ENTRY_FILE

#include "Entry.h"

namespace simparm {

class FileEntry : public StringEntry {
  private:
    std::string getTypeDescriptor() const { return "FileEntry"; }

    NodeHandle create_hidden_node( NodeHandle );
    NodeHandle make_naked_node( NodeHandle );

  public:
   FileEntry(const FileEntry &entry);
   FileEntry(string name, string desc, string value = "");
   ~FileEntry();
    virtual FileEntry* clone() const
        { return new FileEntry(*this); }

   FileEntry &operator=(const string &);

   Attribute<std::string> default_extension;

   operator bool() const;
};

}

#endif
