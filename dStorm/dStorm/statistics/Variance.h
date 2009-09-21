#ifndef DSTORM_VARIANCE_H
#define DSTORM_VARIANCE_H

namespace dStorm {
/** This class implements recursive computation of mean and variance for
 *  a sample. */
class Variance {
  private:
    int count;
    double m, m2;
 
  public:
    Variance() { reset(); }

    inline void addValue(double x) {
        count++;
        double delta = x - m;
        m += delta / count;
        m2 += delta * (x-m);
    }

    double mean() const { return m; }
    double variance() const { return m2/(count-1); }
    int N() const { return count; }

    void reset() { count = 0; m = m2 = 0; }
};
}

#endif
