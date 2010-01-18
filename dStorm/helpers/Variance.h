#ifndef DSTORM_VARIANCE_H
#define DSTORM_VARIANCE_H

namespace dStorm {

/** This class implements on-line computation of a mean . */
template <typename Type = double, typename SizeType = int>
class Mean {
  private:
    SizeType count;
    Type m;

  public:
    Mean() { reset(); }

    inline void addValue( const Type& x ) {
        m += (x - m) / ++count;
    }

    Type mean() const { return m; }
    int N() const { return count; }

    void reset() { count = 0; m = 0; }
};

/** This class implements recursive computation of mean and variance for
 *  a sample. */
template <typename Type = double, 
          typename SquareType = double,
          typename ConvertSizeTo = double>
class Variance {
  private:
    int count;
    Type m; SquareType m2;
 
  public:
    Variance() { reset(); }

    inline void addValue(const Type& x) {
        count++;
        Type delta = x - m;
        m += delta / ConvertSizeTo(count);
        m2 += delta * (x-m);
    }

    Type mean() const { return m; }
    SquareType variance() const { return m2/(count-1); }
    int N() const { return count; }

    void reset() { count = 0; m = 0; m2 = 0; }
};
}

#endif
