/** \file Config.h
 *  This file declares the Config class. */
#ifndef DSTORM_INPUT_CONFIG_H
#define DSTORM_INPUT_CONFIG_H

#include <simparm/Set.hh>
#include <simparm/Entry.hh>
#include "chain/Choice_decl.h"
#include "chain/Link_decl.h"
#include "chain/MetaInfo_decl.h"
#include "chain/Forwarder_decl.h"
#include "FileMethod_decl.h"
#include "Source_decl.h"
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include "chain/Link.h"

/** The dStorm::input namespace provides the functionality to read
 *  sequences of images. The Buffer class provides a simple, vectoresque
 *  view on the input data while storing as little data as possible.
 *  The Config class allows configuration of the input; several methods
 *  are available and selectable there, with a well-defined interface
 *  to add more.
 **/
namespace dStorm {
namespace input {
    class Config
    {
        boost::ptr_list<chain::Link> forwards;

        std::auto_ptr<chain::Link> method;
        class InputChainBase;
        std::auto_ptr<InputChainBase> terminal_node;

      public:
        Config();
        Config(const Config&);
        ~Config();

        Config* clone() const { return new Config(*this); }

        void add_filter( std::auto_ptr<chain::Link> forwarder, bool front = false );
        void add_filter( chain::Link* forwarder, bool front = false ) 
            { add_filter( std::auto_ptr<chain::Link>(forwarder), front ); }
        void add_method( std::auto_ptr<chain::Link> method, chain::Link::Place );
        void add_method( chain::Link* forwarder, chain::Link::Place p ) 
            { add_method( std::auto_ptr<chain::Link>(forwarder), p ); }

        //simparm::Attribute<std::string>& input_file();

        chain::Link& get_link_element();
        const chain::Link& get_link_element() const;
    };
}
}
#endif
