#ifndef DSTORM_INPUT_FAKESOURCE_H
#define DSTORM_INPUT_FAKESOURCE_H

#include <vector>

#include "input/Source.h"

namespace dStorm {
namespace input {

template <typename Type>
class FakeSource : public input::Source<Type> {
    typedef typename input::Source<Type>::TraitsPtr TraitsPtr;
  public:
    FakeSource(const typename input::Traits<Type>& traits,
               const std::vector<Type>& images)
        : traits(new input::Traits<Type>(traits)),
          images(images), current(this->images.begin()) {}

  private:
    bool GetNext(int thread, Type* output) OVERRIDE {
	if (current != images.end()) {
	    *output = *current;
	    ++current;
	    return true;
	} else {
	    return false;
	}
    }

    TraitsPtr get_traits() OVERRIDE { return traits; }

    void attach_ui_( simparm::NodeHandle ) OVERRIDE {}

    void set_thread_count(int num_threads) { assert(num_threads == 1); }
    void dispatch(BaseSource::Messages m) {
        if (m.test(BaseSource::RepeatInput)) {
            current = images.begin();
        }
    }

  private:
    TraitsPtr traits;
    std::vector<Type> images;
    typename std::vector<Type>::const_iterator current;
};

}
}

#endif
