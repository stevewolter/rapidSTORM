#ifndef FITPP_HELPERS_H
#define FITPP_HELPERS_H

namespace fitpp {

template <int Value, template<int> class Functor>
struct ForLoop {
   static void execute(typename Functor<Value>::argument_type& arg)
    {
        ForLoop<Value-1,Functor>::execute(arg);
        Functor<Value>::for_loop_call(arg);
   }
   static void execute(
    typename Functor<Value>::first_argument_type& a1,
    typename Functor<Value>::second_argument_type& a2
   )
    {
        ForLoop<Value-1,Functor>::execute(a1,a2);
        Functor<Value>::for_loop_call(a1,a2);
   }
};
template <template<int> class Functor>
struct ForLoop<-1,Functor> {
    static void execute(typename Functor<0>::argument_type&) {}
    static void execute(typename Functor<0>::first_argument_type&,
                        typename Functor<0>::second_argument_type&)
                        {}
};

template <typename Type, int Rows,int Cols>
struct OptionalMatrix : public Eigen::Matrix<Type,Rows,Cols> {
};

template <typename Type, int Rows> struct OptionalMatrix<Type,Rows,0> 
{ 
    Type& operator[](int) const 
        { assert(false); return *(Type*)NULL; } 
    template <int H,int W>
    Eigen::Matrix<double,H,W> block(int,int) const
        { assert(false); return Eigen::Matrix<double,H,W>::Zero(); }
};
template <typename Type, int Cols> struct OptionalMatrix<Type,0,Cols> 
{ 
    Type& operator[](int) const 
        { assert(false); return *(Type*)NULL; }
    template <int H,int W>
    Eigen::Matrix<double,H,W> block(int,int) const
        { assert(false); return Eigen::Matrix<double,H,W>::Zero(); }
};

};

#endif
