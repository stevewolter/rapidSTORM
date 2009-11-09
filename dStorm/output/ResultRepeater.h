#ifndef DSTORM_RESULT_REPEATER
#define DSTORM_RESULT_REPEATER

namespace dStorm {
namespace output {
    /** Interface for upstream Transmissions that can
     *  repeat the results they have emitted. */
    class ResultRepeater {
      public:
        virtual void repeat_results() = 0;
    };
}
}

#endif
