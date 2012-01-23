#include <memory>

namespace dStorm {
namespace display {

class Manager;
std::auto_ptr<Manager> make_wx_manager();

}
}
