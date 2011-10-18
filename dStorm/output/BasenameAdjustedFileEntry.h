#ifndef DSTORM_OUTPUT_BASENAMEADJUSTED_H
#define DSTORM_OUTPUT_BASENAMEADJUSTED_H

#include <simparm/FileEntry.hh>
#include "BasenameAdjustedFileEntry_decl.h"
#include "Basename.h"

namespace dStorm {
namespace output {

class BasenameAdjustedFileEntry 
: public simparm::FileEntry,
  simparm::Listener
{
  private:
    Basename last_basename;
    bool has_been_user_modified, 
         expect_change,
         is_optional;

  protected:
    void registerNamedEntries();

    void operator()( const simparm::Event& );

  public:
    simparm::Attribute<bool> optional_given;

    BasenameAdjustedFileEntry(
        std::string name,
        std::string desc,
        std::string suffix);
    BasenameAdjustedFileEntry(const BasenameAdjustedFileEntry&);
    ~BasenameAdjustedFileEntry();

    void make_optional();

    void set_output_file_basename
        ( const Basename& basename );
    std::string operator()() const;
    std::string unformatted_name() const { return value(); }

    bool is_given() const { return (!is_optional) || optional_given(); }

    Basename get_basename() const;
};

}
}

#endif
