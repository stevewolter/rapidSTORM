#ifndef DSTORM_ALIGNMENT_FITTER_H
#define DSTORM_ALIGNMENT_FITTER_H

#include <boost/test/unit_test.hpp>
#include <dStorm/Config.h>

std::auto_ptr< dStorm::JobConfig > make_alignment_fitter_config();
boost::unit_test::test_suite* register_alignment_fitter_unit_tests();

#endif
