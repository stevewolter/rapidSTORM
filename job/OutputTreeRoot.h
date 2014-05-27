#ifndef DSTORM_JOB_OUTPUTTREEROOT_H
#define DSTORM_JOB_OUTPUTTREEROOT_H

#include "simparm/TreeRoot.h"
#include "simparm/TreeEntry.h"
#include "output/Config.h"
#include "output/Output.h"
#include "output/FilterSource.h"

namespace dStorm {
namespace job {

class OutputTreeRoot : public output::FilterSource
{
    simparm::TreeRoot tree_root;
    simparm::TreeObject name_object;
    output::Config* my_config;

    std::string getName() const { return name_object.getName(); }
    std::string getDesc() const { return name_object.getDesc(); }
    void attach_ui( simparm::NodeHandle ) { throw std::logic_error("Not implemented on tree base"); }

public:
    OutputTreeRoot();
    OutputTreeRoot( const OutputTreeRoot& other );
    ~OutputTreeRoot() {}

    OutputTreeRoot* clone() const { return new OutputTreeRoot(*this); }
    output::Config &root_factory() { return *my_config; }

    void attach_full_ui( simparm::NodeHandle at );
    void hide_in_tree() { name_object.show_in_tree = false; }
};

}
}

#endif
