#ifndef NONLINFT_TERMINATORS_FWD_H
#define NONLINFT_TERMINATORS_FWD_H

namespace nonlinfit {
/** Contains models of the nonlinfit::Terminator concept. */
namespace terminators {

class RelativeChange;
class StepLimit;

template <class T1, class T2, class C> class Combiner;

}
}

#endif
