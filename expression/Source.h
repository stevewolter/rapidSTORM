#ifndef DSTORM_EXPRESSION_SOURCE_H
#define DSTORM_EXPRESSION_SOURCE_H

#include "SimpleFilters.h"
#include "Config_decl.h"
#include "CommandLine.h"
#include "Source_decl.h"
#include <dStorm/output/Filter.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>
#include "types.h"

namespace dStorm {
namespace expression {

class Source 
: public output::Filter, 
  public simparm::Object,
  public config::ExpressionManager,
  private boost::noncopyable
{
  public:
    typedef expression::Config Config;
  private:
    boost::mutex mutex;
    boost::ptr_vector< config::CommandLine > command_lines;
    SimpleFilters simple_filters;
    boost::ptr_vector< boost::nullable< source::LValue > > expressions;
    std::map< std::string, int > expression_map;
    boost::shared_ptr< Parser > parser;
    Engine* repeater;
    boost::optional<Announcement> my_announcement;

    simparm::Node& getNode() { return *this; }
    const simparm::Node& getNode() const { return *this; }

    Source( const Source& );
    Source* clone() const { return new Source(*this); }
    ~Source();
    AdditionalData announceStormSize(const Announcement&);
    void receiveLocalizations(const EngineResult&);
    void expression_changed( std::string ident, std::auto_ptr<source::LValue> expression );

    bool localization_is_filtered_out( const Localization& ) const;

    void registerNamedEntries();

  public:
    Source( const Config&, std::auto_ptr<output::Output> );
};

}
}

#endif
