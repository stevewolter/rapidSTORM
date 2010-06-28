#ifndef FITPP_PARAMETERALLOCATOR_H
#define FITPP_PARAMETERALLOCATOR_H

#include <fit++/Position.hh>
#include <fit++/Helpers.hh>
#include <fit++/BitfieldConstructor.hh>

namespace fitpp {

template <int Globals, int _Kernels, int Locals, int ParameterMask>
struct ParamMap {
    static const int Kernels = _Kernels;
    static const int GlobalMask = (0x1 << Globals) - 1;

    static const int ParamC = Globals + Locals * Kernels;
    static const int GlobalVarC = Bits<ParameterMask & GlobalMask>::Count;
    static const int GlobalConstC = Globals - GlobalVarC;
    static const int LocalVarC = Bits<ParameterMask & (~GlobalMask)>::Count;
    static const int VarC = GlobalVarC + Kernels * LocalVarC;
    static const int ConstC = ParamC - VarC;

    typedef OptionalMatrix<double,ConstC,1> Constants; 
    typedef typename fitpp::Position<VarC>::Vector Variables;

    template <int Number>
    class Parameter {
        static const int ConstOffset = (Globals - GlobalVarC),
                        ConstStep = (Locals - LocalVarC);
        static const int VarOffset = GlobalVarC, VarStep = LocalVarC;
      public:
        static const bool Global = (Number < Globals), 
                        Variable = (ParameterMask & (1 << Number));

        static const int ParamOffset = Number;
        static const int ParamStep = (Global) ? 0 : Locals;

      private:
        static const int Vars_before_this 
            = Bits<ParameterMask & ((1 << Number)-1)>::Count;
        static const int Elements_before_this
            = (Variable) ?  Vars_before_this : Number - Vars_before_this;
        static const int Global_elements_before_this
            = (Global) ? Number : ((Variable) ? GlobalVarC : GlobalConstC);
        static const int Local_Elements_before_this
            = Elements_before_this - Global_elements_before_this;

        static const int Offset = Global_elements_before_this;
        static const int Step = (Global) ? 0 : 1;
        static const int Stride = 
            (Global) ? 0 : Local_Elements_before_this;

      public:
        static const int PositionInKernel = 
            (Global) ? Local_Elements_before_this : Global_elements_before_this;
        template <int Kernel>
        struct InKernel {
            static const int N = Stride * Kernels + Step * Kernel + Offset;
        };

        typedef InKernel<0> AsGlobal;

        template <typename Fitter>
        static void set_absolute_epsilon( Fitter& fitter, double value ) {
            set_absolute_epsilon_recursive<Fitter,Kernels-1>
                (fitter, value);
        }

        static void set_all( Variables& v, Constants& c, double t ) {
            for (int i = 0; i < Kernels; ++i ) {
                if ( Variable )
                    v[Stride * Kernels + Step * i + Offset] = t;
                else
                    c[Stride * Kernels + Step * i + Offset] = t;
            }
        }
      private:
        template <class Fitter, int Kernel>
        static void set_absolute_epsilon_recursive( 
            Fitter& fitter, double value
        ) {
            fitter.set_absolute_epsilon
                ( InKernel<Kernel>::N, value );
            if ( Kernel > 0 )
                set_absolute_epsilon_recursive<Fitter,
                                    (Kernel>0) ? Kernel - 1 : 0>
                    (fitter, value);
        }

    };

    template <int Name, int Kernel>
    static double& value(Variables &v, Constants& c) {
        typedef typename Parameter<Name>::template InKernel<Kernel> InKernel;
        if ( Parameter<Name>::Variable ) 
            return v[InKernel::N]; 
        else 
            return c[InKernel::N];
    }

    template <int Name, int Kernel>
    static double value(const Variables &v, const Constants& c) {
        typedef typename Parameter<Name>::template InKernel<Kernel> InKernel;
        if ( Parameter<Name>::Variable ) 
            return v[InKernel::N]; 
        else 
            return c[InKernel::N];
    }
};

}

#endif
