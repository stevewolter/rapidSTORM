#ifndef LOCPREC_COUNTER_H
#define LOCPREC_COUNTER_H

namespace locprec {
    class Counter {
      private:
        int val;
      public:
        Counter() : val(0) {}
        inline void operator++() { val++; }
        inline void operator++(int) { val++; }
        inline operator int() const { return val; }
        inline void operator=(const Counter& c) { val = c.val; }
        inline void operator=(int v) { val = v; }
    };
}

#endif
