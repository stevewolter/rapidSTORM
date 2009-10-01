#include <Eigen/Core>

namespace fitpp {
    template <typename Enum, int VarC, int NumFields> 
    class BitfieldConstructor {
      private:
        typedef Eigen::Matrix<bool,NumFields,1> Vector;
        Vector params;

      public:
        BitfieldConstructor(const Vector& params) throw()
            : params(params) {}
        BitfieldConstructor<Enum,VarC-1,NumFields> operator()(Enum n) 
            throw()
        { 
            params(n) = true;
            return BitfieldConstructor<Enum,VarC-1,NumFields>(params);
        }

        BitfieldConstructor() throw() 
            : params(Vector::Constant(false)) {}
    };
    template <typename Enum, int NumFields>
    class BitfieldConstructor<Enum, 0, NumFields> {
      public:
        typedef Eigen::Matrix<bool,NumFields,1> Vector;
      private:
        Vector params;
      public:
        BitfieldConstructor(const Vector& params) throw()
            : params(params) {}
        BitfieldConstructor() throw() 
            : params(Vector::Constant(false)) {}
        const Vector& operator()() const throw() 
            { return params; }
    };

    template <int i> struct Bits {
        static const int Count = (i % 2) + Bits< i/2 >::Count; 
    };
    template <> struct Bits<0> { static const int Count = 0; };

    template <int Field, int Number> struct Nth_One_Bit {
        static const int Position =
                Nth_One_Bit<Field/2,Number-(Field%2)>::Position + 1;
    };

    template <int Field> struct Nth_One_Bit<Field,-1> {
        static const int Position = -1;
    };
}
