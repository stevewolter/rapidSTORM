#include "estimate_psf_form/VariableReduction_test.h"
#include "estimate_psf_form/VariableReduction.h"

#include <boost/mpl/joint_view.hpp>
#include "gaussian_psf/DepthInfo3D.h"
#include "gaussian_psf/No3D.h"

namespace dStorm {
namespace estimate_psf_form {

static void test_no3d() {
    Config config;
    config.disjoint_amplitudes = true;
    config.fit_prefactors = false;
    VariableReduction<typename boost::mpl::joint_view<
        gaussian_psf::No3D::Variables,
        constant_background::Expression::Variables>::type> reduction(config, 3, true, 12);
    reduction.add_plane(0, 0);
    reduction.add_plane(1, 0);
    reduction.add_plane(2, 0);
    reduction.add_plane(0, 2);
    reduction.add_plane(1, 2);
    reduction.add_plane(2, 2);
    reduction.add_plane(0, 1);
    reduction.add_plane(1, 1);
    reduction.add_plane(2, 1);
    reduction.add_plane(0, 2);
    reduction.add_plane(1, 2);
    reduction.add_plane(2, 2);

    nonlinfit::sum::VariableMap variable_map(reduction.get_reduction_matrix());
    std::vector<std::vector<int>> expected_map{
        { -1, -1, 0, 1, 2, -1, 3, 3, 4 },
        { -1, -1, 0, 1, 5, -1, 6, 6, 7 },
        { -1, -1, 0, 1, 8, -1, 9, 9, 10 },
        { -1, -1, 11, 12, 13, -1, 14, 14, 15 },
        { -1, -1, 11, 12, 16, -1, 17, 17, 18 },
        { -1, -1, 11, 12, 19, -1, 20, 20, 21 },
        { -1, -1, 22, 23, 24, -1, 25, 25, 26 },
        { -1, -1, 22, 23, 27, -1, 28, 28, 29 },
        { -1, -1, 22, 23, 30, -1, 31, 31, 32 },
        { -1, -1, 33, 34, 35, -1, 14, 14, 36 },
        { -1, -1, 33, 34, 37, -1, 17, 17, 38 },
        { -1, -1, 33, 34, 39, -1, 20, 20, 40 } };
    bool all_equal = true;
    for (size_t row = 0; row < expected_map.size(); ++row) {
        for (size_t col = 0; col < expected_map[row].size(); ++col) {
            bool entry_equal = expected_map[row][col] == variable_map(row, col);
            if (!entry_equal) {
                std::cerr << "Entry " << row << "," << col <<
                    " mismatches: Expected " << expected_map[row][col] <<
                    ", got " << variable_map(row, col) << "\n";
            }
            all_equal = all_equal && entry_equal;
        }
    }
    BOOST_CHECK(all_equal);
}

static void test_depthinfo3d() {
    Config config;
    config.disjoint_amplitudes = false;
    config.laempi_fit = true;
    config.fit_prefactors = true;
    config.circular_psf = false;
    VariableReduction<typename boost::mpl::joint_view<
        gaussian_psf::DepthInfo3D::Variables,
        constant_background::Expression::Variables>::type> reduction(config, 3, true, 8);
    reduction.add_plane(0, 0);
    reduction.add_plane(1, 0);
    reduction.add_plane(0, 2);
    reduction.add_plane(1, 2);
    reduction.add_plane(0, 1);
    reduction.add_plane(1, 1);
    reduction.add_plane(0, 2);
    reduction.add_plane(1, 2);

    nonlinfit::sum::VariableMap variable_map(reduction.get_reduction_matrix());
    std::vector<std::vector<int>> expected_map{
        { -1, -1, 0, 1, 2, 3, 4, 5 },
        { -1, -1, 6, 7, 2, 8, 4, 9 },
        { -1, -1, 10, 11, 12, 13, 14, 15 },
        { -1, -1, 16, 17, 12, 18, 14, 19 },
        { -1, -1, 20, 21, 22, 23, 24, 25 },
        { -1, -1, 26, 27, 22, 28, 24, 29 },
        { -1, -1, 30, 31, 32, 13, 33, 34 },
        { -1, -1, 35, 36, 32, 18, 33, 37 },
    };
    BOOST_CHECK_EQUAL(expected_map.size(), variable_map.function_count());
    bool all_equal = true;
    for (size_t row = 0; row < expected_map.size(); ++row) {
        for (size_t col = 0; col < expected_map[row].size(); ++col) {
            bool entry_equal = expected_map[row][col] == variable_map(row, col);
            if (!entry_equal) {
                std::cerr << "Entry " << row << "," << col <<
                    " mismatches: Expected " << expected_map[row][col] <<
                    ", got " << variable_map(row, col) << "\n";
            }
            all_equal = all_equal && entry_equal;
        }
    }
    BOOST_CHECK(all_equal);
}

boost::unit_test::test_suite* test_VariableReduction() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "VariableReduction" );
    rv->add( BOOST_TEST_CASE(&test_no3d) );
    rv->add( BOOST_TEST_CASE(&test_depthinfo3d) );
    return rv;
}

}
}
