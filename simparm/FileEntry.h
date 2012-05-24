#ifndef SIMPARM_CONFIG_ENTRY_FILE
#define SIMPARM_CONFIG_ENTRY_FILE

#include "Entry.h"

namespace simparm {

class FileEntry : public StringEntry {
  private:
    std::string getTypeDescriptor() const { return "FileEntry"; }
    std::auto_ptr<std::ofstream> openedOStream;
    std::auto_ptr<std::ifstream> openedIStream;
    std::ostream* out_stream;
    std::istream* in_stream;

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

   std::ostream& get_output_stream(bool append = false);
   void close_output_stream();
   std::istream& get_input_stream();
   void close_input_stream();
};

}

#endif
