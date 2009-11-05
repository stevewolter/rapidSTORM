#ifndef TRANSMISSIONFACTORY_H
#define TRANSMISSIONFACTORY_H

#include <dStorm/output/Output.h>
#include <memory>
#include <simparm/Object.hh>
#include <simparm/TreeEntry.hh>
#include <set>

namespace simparm { class FileEntry; }

namespace dStorm {

/** An OutputSource object is a config that produces
 *  output objects. It is used for forwarding as well
 *  as leaf outputs and controls the config tree and
 *  the basename setting. Implementors must override the
 *  make_output() method and can override the
 *  set_output_file_basename method. */
class OutputSource 
: public simparm::TreeAttributes
{
  protected:
    class AdjustedList;
    std::auto_ptr<AdjustedList> adjustedList;

    OutputSource();
    OutputSource(const OutputSource& o);

  public:
    simparm::Attribute<std::string> help_file;

    void adjust_to_basename(simparm::FileEntry&);

    virtual ~OutputSource();
    virtual OutputSource* clone() const = 0;

    /** \return A suitable Output object with
     *          all outputs configured. \note The root output
     *          will be constructed last in a tree of
     *          OutputSource objects. */
    virtual std::auto_ptr<Output> make_output() 
 = 0;

    static const int Basename_Accepted = 0;
    static const int Basename_Conflicted = 1;

    typedef int BasenameResult;
    /** Notification for new input file basename. \c avoid_name
     *  is the name of the input file; that name should be avoided,
     *  as it would overwrite our input. */
    virtual BasenameResult set_output_file_basename(
        const std::string& new_basename, std::set<std::string>& avoid)
;

    virtual std::string getDesc() const = 0;
};

}

#endif
