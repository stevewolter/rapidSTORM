#ifndef DSTORM_EXPRESSION_SOURCE_H
#define DSTORM_EXPRESSION_SOURCE_H

#include "expression/SimpleFilters.h"
#include "expression/Config_decl.h"
#include "CommandLine.h"
#include "expression/Source_decl.h"
#include "output/Filter.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>
#include "expression/types.h"

namespace dStorm {
namespace expression {

class Source 
: public output::Filter, 
  public config::ExpressionManager,
  private boost::noncopyable
{
  private:
    boost::mutex mutex;
    boost::ptr_vector< config::CommandLine > command_lines;
    SimpleFilters simple_filters;
    boost::ptr_vector< boost::nullable< source::LValue > > expressions;
    std::map< std::string, int > expression_map;
    boost::shared_ptr< Parser > parser;
    Engine* repeater;
    boost::optional<Announcement> my_announcement;

    ~Source();
    void announceStormSize(const Announcement&) OVERRIDE;
    void receiveLocalizations(const EngineResult&);
    void expression_changed( std::string ident, std::auto_ptr<source::LValue> expression );

    bool localization_is_filtered_out( const Localization& ) const;

    void attach_ui_( simparm::NodeHandle );

  public:
    Source( const Config&, std::auto_ptr<output::Output> );
};

}
}

#endif
