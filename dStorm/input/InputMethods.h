#ifndef DSTORM_INPUT_INPUTMETHODS_H
#define DSTORM_INPUT_INPUTMETHODS_H

#include <memory>
#include <dStorm/input/chain/Choice.h>

namespace dStorm {
namespace input {

class InputMethods 
: public chain::Choice 
{
    std::auto_ptr< chain::Link > file_method;
  public:
    InputMethods();
    InputMethods(const InputMethods&);
    ~InputMethods();
    InputMethods* clone() const { return new InputMethods(*this); }
    void insert_new_node( std::auto_ptr<chain::Link> );
};

}
}

#endif
