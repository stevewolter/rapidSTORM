#ifndef SIMPARM_TEXT_STREAM_NODE_H
#define SIMPARM_TEXT_STREAM_NODE_H

#include "../Node.hh"
#include "../BaseAttribute.hh"
#include <boost/signals2/signal.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace simparm {
namespace text_stream {
template<typename T>
struct maximum
{
  typedef T result_type;

  template<typename InputIterator>
  T operator()(InputIterator first, InputIterator last) const
  {
    if(first == last ) return false;
    T max_value = *first++;
    while (first != last) {
        if (max_value < *first)
            max_value = *first;
        ++first;
    }

    return max_value;
  }
};

struct Node : public simparm::Node {
    std::string name, type;
    boost::signals2::signal< bool( std::string ), maximum<bool> > print_, print_top_level_;
    std::vector< Node* > nodes;
    std::map< std::string, Node* > node_lookup;
    std::vector< BaseAttribute* > attributes;
    std::map< std::string, BaseAttribute* > attribute_lookup;
    boost::ptr_vector< boost::signals2::scoped_connection > connections;
    bool declared;

    void print_attribute_value( const simparm::BaseAttribute& );
    void process_attribute( BaseAttribute&, std::istream& );

protected:
    virtual bool print( const std::string& );
    virtual bool print_on_top_level( const std::string& );

    void listen_to( Node& o );
    void show_children();
    void show_attributes( std::ostream& );
    void declare(std::ostream&);
    void undeclare();

    Node( std::string name, std::string type ) : name(name), type(type), declared(false) {}
    std::auto_ptr<simparm::Node> create_node( std::string name, std::string type );

public:
    std::auto_ptr<simparm::Node> create_object( std::string name );
    std::auto_ptr<simparm::Node> create_entry( std::string name, std::string desc, std::string type );
    std::auto_ptr<simparm::Node> create_set( std::string name );
    std::auto_ptr<simparm::Node> create_choice( std::string name, std::string desc );
    std::auto_ptr<simparm::Node> create_file_entry( std::string name, std::string desc );
    std::auto_ptr<simparm::Node> create_progress_bar( std::string name, std::string desc );
    std::auto_ptr<simparm::Node> create_trigger( std::string name, std::string desc );
    void add_attribute( simparm::BaseAttribute& );
    void send( Message& m );
    void show();
    void hide();
    /** TODO: Method is deprecated and should be removed on successful migration. */
    bool isActive() const;

    void processCommand( std::istream& );
    virtual void processCommand( const std::string&, std::istream& );
};

}
}

#endif
