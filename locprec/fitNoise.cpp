#include <simparm/Set.hh>
#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <simparm/FileEntry.hh>
#include <gsl/gsl_statistics_double.h>
#include <gsl/gsl_multifit_nlin.h>
#include <math.h>
#include <algorithm>
#include "foreach.h"
#include <gsl/gsl_blas.h>
#include <map>
#include <vector>
#include <string.h>
#include <fstream>

using namespace std;
using namespace simparm;

struct FitData { vector<unsigned long> values;
                 double start;
                 double stride;
                    vector<double> funcs[2]; };

static int gauss_fdf(const gsl_vector *x, void *vdata,
                        gsl_vector *f, gsl_matrix *J)
{
    FitData &fitData = *(FitData*)vdata;
    double x0 = gsl_vector_get(x, 0);
    double sigma = gsl_vector_get(x, 1);
    double a = gsl_vector_get(x, 2);

    double vf = 1 / ( sqrt(2 * M_PI) * sigma);
    for (unsigned int vector_index = 0; vector_index < fitData.values.size();
            vector_index++) 
    {
        double x = vector_index * fitData.stride + fitData.start;
        double xt = x - x0, xterm = (xt*xt) / (sigma*sigma);
        double e = a * vf * exp( - 1.0/2 * xterm );
        fitData.funcs[0][vector_index] = e;

        if (f != NULL)
            gsl_vector_set(f, vector_index, e - fitData.values[vector_index]);
        if (J != NULL) {
            gsl_matrix_set(J, vector_index, 0, (xt * e) / (sigma*sigma));
            gsl_matrix_set(J, vector_index, 1, (xterm-1) * (e/sigma));
            gsl_matrix_set(J, vector_index, 2, e / a);
        }
    }
    return GSL_SUCCESS;
}

static int gauss_f(const gsl_vector *x, void *data, gsl_vector *f)
    { return gauss_fdf(x, data, f, NULL); }
static int gauss_df(const gsl_vector *x, void *data, gsl_matrix *J)
    { return gauss_fdf(x, data, NULL, J); }

static int combined_fdf(const gsl_vector *x, void *vdata,
                        gsl_vector *f, gsl_matrix *J)
{
    FitData &fitData = *(FitData*)vdata;
    double theta = gsl_vector_get(x, 0);
    double amp = gsl_vector_get(x, 1);
    double xo = gsl_vector_get(x, 2);
    double x0 = gsl_vector_get(x, 3);
    double sigma = gsl_vector_get(x, 4);
    double a = gsl_vector_get(x, 5);

    double vf = 1 / ( sqrt(2 * M_PI) * sigma);
    for (unsigned int vector_index = 0; vector_index < fitData.values.size(); 
            vector_index++) 
    {
        double x = vector_index * fitData.stride + fitData.start;
        double cx = (x - xo);
        double gamma_part = 0;
        double xt = x - x0, xterm = (xt*xt) / (sigma*sigma);
        double nd = vf * exp( - 1.0/2 * xterm );
        double e = a * nd;

        if (cx < 0) {
            if (J != NULL) {
                gsl_matrix_set(J, vector_index, 0, 0);
                gsl_matrix_set(J, vector_index, 1, 0);
                gsl_matrix_set(J, vector_index, 2, 0);
            }
        } else {
            double red = exp( - cx / theta ) / (theta*theta);
            double val = cx * red;
            double av = amp * val;

            gamma_part = av;
            if (J != NULL) {
                gsl_matrix_set(J, vector_index, 0, (av/theta) * 
                                            (cx / theta - 2 ));
                gsl_matrix_set(J, vector_index, 1, val );
                gsl_matrix_set(J, vector_index, 2, amp * (val / theta - red));
            }
        } 

        fitData.funcs[1][vector_index] = e + gamma_part;
        if (f != NULL)
            gsl_vector_set(f, vector_index, e + gamma_part - 
                                            fitData.values[vector_index]);
        if (J != NULL) {
            gsl_matrix_set(J, vector_index, 3, (xt * e) / (sigma*sigma));
            gsl_matrix_set(J, vector_index, 4, (xterm-1) * (e/sigma));
            gsl_matrix_set(J, vector_index, 5, nd );
        }
    }
    return GSL_SUCCESS;
}
static int combined_f(const gsl_vector *x, void *data, gsl_vector *f)
    { return combined_fdf(x, data, f, NULL); }
static int combined_df(const gsl_vector *x, void *data, gsl_matrix *J)
    { return combined_fdf(x, data, NULL, J); }


void readNoise(const char *filename, vector<unsigned long>& values, int stride) 
{
    auto_ptr<ifstream> filestream_deallocator;
    istream *stream = &cin;
    if ( strcmp( filename, "-" ) != 0 ) {
        filestream_deallocator.reset(new ifstream(filename, ios_base::in));
        stream = filestream_deallocator.get();
    }

    while ( *stream ) {
        unsigned int position, count;
        (*stream) >> position >> count;
        position /= stride;
        if (*stream) {
            if (values.size() <= position) values.resize(position+1, 0);
            values[position] = count;
        }
    }
}

int main(int argc, char *argv[]) {
    Set cs("", "");
    UnsignedLongEntry adNoise("ADNoise", 
                        "ADNoise used in the NoiseMeter", 8);
    cs.register_entry(&adNoise);
    FileEntry fileName("InputFile", "");
    cs.register_entry(&fileName);
    cs.readConfig(argc, argv);

    FitData fitData;
    fitData.start = 0;
    fitData.stride = adNoise();
    readNoise(fileName().c_str(), fitData.values, adNoise());
    fitData.values.resize(fitData.values.size() + fitData.values.size()/2, 0);
    fitData.funcs[0].resize(fitData.values.size(), 0);
    fitData.funcs[1].resize(fitData.values.size(), 0);
    
    unsigned int maxIndex = 0, maxCount = 0, firstNonzero = 40000;
    double total = 0;

    for( unsigned int i = 0; i < fitData.values.size(); i++) {
        if ( fitData.values[i] > maxCount ) {
            maxCount = fitData.values[i];
            maxIndex = i;
        }
        if ( fitData.values[i] > 0 && i < firstNonzero )
            firstNonzero = i;
        total += fitData.values[i];
    }

    int np = 6;
    const gsl_multifit_fdfsolver_type *solver_type = 
            gsl_multifit_fdfsolver_lmsder;
    gsl_multifit_fdfsolver * solver_gauss = 
        gsl_multifit_fdfsolver_alloc(solver_type, fitData.values.size(), 3),
                                *solver_combined = 
        gsl_multifit_fdfsolver_alloc(solver_type, fitData.values.size(), np);
    gsl_multifit_function_fdf fit_function_gauss;
    fit_function_gauss.f = gauss_f;
    fit_function_gauss.df = gauss_df;
    fit_function_gauss.fdf = gauss_fdf;
    fit_function_gauss.p = 3;
    gsl_multifit_function_fdf fit_function_combined;
    fit_function_combined.f = combined_f;
    fit_function_combined.df = combined_df;
    fit_function_combined.fdf = combined_fdf;
    fit_function_combined.p = np;

    fit_function_gauss.n = fit_function_combined.n = fitData.values.size();
    fit_function_gauss.params = fit_function_combined.params =
        &fitData;

    gsl_vector *gauss_parameters = gsl_vector_alloc( 3 );
    gsl_vector *combined_parameters = gsl_vector_alloc( np );
    gsl_vector_set(gauss_parameters, 0, maxIndex * adNoise());
    gsl_vector_set(gauss_parameters, 1, 10 * adNoise());
    gsl_vector_set(gauss_parameters, 2, total);
    gsl_multifit_fdfsolver_set( solver_gauss, &fit_function_gauss,
                                gauss_parameters );

    for (int i = 0; i < 100; i++) {
        gsl_multifit_fdfsolver_iterate( solver_gauss );
    }

    gsl_vector_set(combined_parameters, 0, 
        gsl_vector_get(solver_gauss->x, 1) );
    gsl_vector_set(combined_parameters, 1, 0);
    gsl_vector_set(combined_parameters, 2, firstNonzero * adNoise());
    cerr << firstNonzero * adNoise() << endl;
    gsl_vector_set(combined_parameters, 3, 
        gsl_vector_get(solver_gauss->x, 0));
    gsl_vector_set(combined_parameters, 4, 
        gsl_vector_get(solver_gauss->x, 1));
    gsl_vector_set(combined_parameters, 5, 
        gsl_vector_get(solver_gauss->x, 2));

    gsl_multifit_fdfsolver_set( solver_combined,
        &fit_function_combined, combined_parameters );

    for (int i = 0; i < 100; i++) {
        double cres;
        gsl_multifit_fdfsolver_iterate( solver_combined );
        gsl_blas_ddot( solver_combined->f, solver_combined->f, &cres );
        cerr << "Residues are " << cres << endl;
    }
    for( unsigned int i = 0; i < fitData.values.size(); i++) {
        cout << i * fitData.stride + fitData.start
                << " " << fitData.values[i]
                << " " << fitData.funcs[0][i]
                << " " << fitData.funcs[1][i]
                << endl;
    }

    std::cerr 
        << "Estimates of the ND parameters of the noise are"
        << " mean " << gsl_vector_get(solver_gauss->x, 0)
        << " deviation " << gsl_vector_get(solver_gauss->x, 1)
        << " amplitude " << gsl_vector_get(solver_gauss->x, 2)
        << " with total pixel number " << total
        << std::endl;

    std::cerr 
        << "Estimates of the CD parameters of the noise are "
        << " theta " << gsl_vector_get(solver_combined->x, 0)
        << " gm_amp " << gsl_vector_get(solver_combined->x, 1)
        << " gm_offset " << gsl_vector_get(solver_combined->x, 2)
        << " gs_offset " << gsl_vector_get(solver_combined->x, 3)
        << " sigma " << gsl_vector_get(solver_combined->x, 4)
        << " gs_amp " << gsl_vector_get(solver_combined->x, 5)
        << " with total pixel number " << total
        << std::endl;

    double gauss_res, cres;
    gsl_blas_ddot( solver_gauss->f, solver_gauss->f, &gauss_res );
    gsl_blas_ddot( solver_combined->f, solver_combined->f, &cres );
    std::cerr << "Residues are " << gauss_res << " for gauss and "
                << cres << " for combined approximation." << std::endl;

    gsl_vector_free(gauss_parameters);
    gsl_multifit_fdfsolver_free(solver_gauss);
    gsl_vector_free(combined_parameters);
    gsl_multifit_fdfsolver_free(solver_combined);
}
