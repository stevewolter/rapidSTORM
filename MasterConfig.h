#ifndef DSTORM_MASTERCONFIG_H
#define DSTORM_MASTERCONFIG_H

#include <simparm/Node.hh>
#include <boost/shared_ptr.hpp>
#include "engine/CarConfig.h"
#include <boost/thread.hpp>

namespace dStorm {

class MasterConfig
{
    boost::mutex usage_count_mutex;
    boost::condition_variable usage_count_is_one;
    short usage_count;
    bool may_be_referenced;

  public:
    class Ptr {
      private:
        friend class MasterConfig;
        mutable MasterConfig& m;
        Ptr( MasterConfig* mc );

      public:
        Ptr( const Ptr& );
        ~Ptr();

        MasterConfig& operator*() const { return m; }
        MasterConfig* operator->() const { return &m; }
        MasterConfig* get() { return &m; }

        void wait_for_exclusive_ownership();
    };

    static Ptr create();
    virtual ~MasterConfig();

    virtual void thread_safely_register_node( simparm::Node& node ) = 0;
    virtual void thread_safely_erase_node( simparm::Node& node ) = 0;

    virtual void add_modules( engine::CarConfig& config ) = 0;

    virtual void read_input(std::istream&) = 0;
};

}

#endif
