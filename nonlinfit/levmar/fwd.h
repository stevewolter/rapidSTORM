#ifndef NONLINFIT_LEVENBERG_MARQUARDT_DECL_H
#define NONLINFIT_LEVENBERG_MARQUARDT_DECL_H

namespace nonlinfit {
/** Contains an implementation of the Levenberg-Marquardt fitting method. */
namespace levmar {

struct InvalidStartPosition;
struct FoundNoInitialStep;
struct SingularMatrix;
struct Config;
class Fitter;
class FitResult;

}
}

#endif
