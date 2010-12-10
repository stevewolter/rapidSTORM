#ifndef LOCPREC_COUNTER_H
#define LOCPREC_COUNTER_H

namespace locprec {
    class Counter {
      private:
        int val;
      public:
        Counter() : val(0) {}
        inline void operator++() throw() { val++; }
        inline void operator++(int) throw() { val++; }
        inline operator int() const throw() { return val; }
        inline void operator=(const Counter& c) throw() { val = c.val; }
        inline void operator=(int v) throw() { val = v; }
    };
}

#endif
