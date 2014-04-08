#ifndef SIMPARM_OBJECT_HH
#define SIMPARM_OBJECT_HH

#include <string>
#include "simparm/NodeHandle.h"
#include "simparm/UserLevel.h"
#include <memory>

namespace simparm {

class Object {
  public:
    Object(std::string name, std::string desc);
    Object(std::string name);
    Object( const Object& );
    ~Object();

    std::string getName() const;
    std::string getDesc() const;

    NodeHandle attach_ui( simparm::NodeHandle node );
    void detach_ui( simparm::NodeHandle node );

    NodeHandle get_user_interface_handle() const { return node_; }

    void show();
    void hide();
    void set_visibility( bool is_visible );
    void set_user_level( UserLevel );

protected:
    virtual NodeHandle create_hidden_node( simparm::NodeHandle );
    virtual NodeHandle make_naked_node( simparm::NodeHandle );
private:
    NodeHandle node_;
    std::string name, desc;
    bool is_visible;
    UserLevel user_level;
};

}

#endif
