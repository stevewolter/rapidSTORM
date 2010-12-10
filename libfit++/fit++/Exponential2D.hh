#ifndef LIBFITPP_EXPONENTIAL2D_H
#define LIBFITPP_EXPONENTIAL2D_H

#include <fit++/LatticeFitFunction.hh>

#include <iostream>
using namespace std;

namespace libfitpp {

    /** The Exponential2D class fits the given lattice points with an
     *  exponential model of the form 
     *  A * exp( -1/2 * (x-x0)^2 / sigma_x^2 * (y-y0)^2 / sigma_y^2 ) + B.
     *
     *  All of these parameters must be declared as fixed or fitted
     *  at construction time. */
    template <typename DataPoint, int VariableNumber>
    class Exponential2D 
    : public FitFunction<VariableNumber>,
      public LatticeFitFunction<DataPoint> 
    {
      public:
        /** The Parameters enumeration enumerates all model parameters.
         *  NoParameter terminates the enumeration, and a loop from
         *  FirstParam to LastParam (inclusive) yields all parameters. */
        enum Parameters { MeanX = 0 /** x0 */, MeanY = 1 /** y0 */,
                          Amplitude = 2 /** A */, 
                          SigmaX = 3 /** sigma_x */, 
                          SigmaY = 4 /** sigma_y */, 
                          SigmaXY = 5, /** Correlation between axes */
                          Shift = 6 /** B */, NoParameter,
                          FirstParam = MeanX, LastParam = Shift };

        /** The constructor.
         *  
         *  \param nx Lattice width
         *  \param ny Lattice height
         */
        Exponential2D () throw ();
        /** Destructor */
        ~Exponential2D() throw();

        inline void setLimitForNegligibleCorrelation(double val) throw()
            { correlationNegligibleLimit = val; }

        inline void setMeanX(double v) throw()
            { setParameter(MeanX, v); }
        inline double getMeanX() throw()
            { return getParameter(MeanX); }

        inline void setMeanY(double v) throw()
            { setParameter(MeanY, v); }
        inline double getMeanY() throw()
            { return getParameter(MeanY); }

        inline void setAmplitude(double v) throw()
            { setParameter(Amplitude, v); }
        inline double getAmplitude() throw()
            { return getParameter(Amplitude); }

        inline void setShift(double v) throw()
            { setParameter(Shift, v); }
        inline double getShift() throw()
            { return getParameter(Shift); }

        inline void setSigmaX(double v) throw()
            { setParameter(SigmaX, v); }
        inline double getSigmaX() throw()
            { return getParameter(SigmaX); }

        inline void setSigmaY(double v) throw()
            { setParameter(SigmaY, v); }
        inline double getSigmaY() throw()
            { return getParameter(SigmaY); }

        inline void setSigmaXY(double v) throw()
            { setParameter(SigmaXY, v); }
        inline double getSigmaXY() throw()
            { return getParameter(SigmaXY); }

      protected:
        double correlationNegligibleLimit;

        typedef typename FitFunction<VariableNumber>::Gradient VarVec;
        virtual bool computeDerivatives(
            double &residuesSq, VarVec& gradient, 
            typename FitFunction<VariableNumber>::Curvature& redHess)
            throw();

        bool changeIsNegligible(const VarVec& position, const VarVec &step)
            throw() 
        {
            return (step.cwise() / position).cwise().abs().maxCoeff()<0.01;
        }
    };

    template <typename DataType>
    class Exponential2D_FixedForm : public Exponential2D<DataType,4> 
    {
      private:
        double minChange;
      public:
        Exponential2D_FixedForm(double minChange) throw() 
        : Exponential2D<DataType,4>(), minChange(minChange)
        {
            this->setVariableness(this->MeanX, true);
            this->setVariableness(this->MeanY, true);
            this->setVariableness(this->Amplitude, true);
            this->setVariableness(this->Shift, true);
        }
        
        bool changeIsNegligible
            (const Eigen::Vector4d&, const Eigen::Vector4d &step) throw() 
        {
            double x = abs( step[ this->parameterIndex[this->MeanX] ] );
            double y = abs( step[ this->parameterIndex[this->MeanY] ] );
            return (x < minChange) && (y < minChange);
        }
    };

    template <typename DataType>
    class Exponential2D_FixedCenter : public Exponential2D<DataType,5> 
    {
      private:
        double minChange;
      public:
        Exponential2D_FixedCenter(double minChange)
        throw() : Exponential2D<DataType,5>(), minChange(minChange)
        {
            for (int i = this->FirstParam; i <= this->LastParam; i++)
                this->setVariableness(i, 
                            (i != this->MeanX && i != this->MeanY));
        }

        bool changeIsNegligible(const FitFunction<5>::Gradient &,
            const FitFunction<5>::Gradient &step) throw() 
        {
            double x, y, xy;
            x = abs( step[ this->parameterIndex[this->SigmaX] ] );
            y = abs( step[ this->parameterIndex[this->SigmaX] ] );
            xy = abs( step[ this->parameterIndex[this->SigmaXY] ] );
            return (x < minChange) && (y < minChange) && (xy < minChange);
        }
    };

    template <typename DataType>
    class Exponential2D_Free : public Exponential2D<DataType,7> 
    {
      public:
        Exponential2D_Free() throw() 
        : Exponential2D<DataType,7>() 
        {
            for (int i = this->FirstParam; i <= this->LastParam; i++)
                this->setVariableness(i, true);
        }

    };
}
#endif
