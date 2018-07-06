#include "kalman_filter/EmissionTracker_test.h"

#include "localization/Traits.h"
#include "kalman_filter/EmissionTracker.h"
#include "output/LocalizedImage.h"
#include "output/LocalizedImage_traits.h"

namespace dStorm {
namespace kalman_filter {
namespace emission_tracker {

class FakeOutput : public output::Output {
  public:
    FakeOutput() {}

    void announceStormSize(const Announcement& a) OVERRIDE {
        announcement.reset(new Announcement(a));
    }

    void receiveLocalizations(const EngineResult& r) OVERRIDE {
        results.push_back(r);
    }

    std::unique_ptr<Announcement> announcement;
    std::vector<EngineResult> results;
};

struct TestLocalization {
    float x, y;
    int t;
    int molecule;
    int offset_in_molecule;
};

std::vector<std::pair<int, std::vector<TestLocalization>>> testdata = {
    {0, {{100.0, 500.0, 0, 0, 0}}},
    {1, {{105.0, 495.0, 1, 0, 1}, {205.0, 395.0, 1, 1, 0}}},
    {2, {{95.0, 505.0, 2, 0, 2}}},
    {3, {}},
    {4, {{95.0, 505.0, 4, 0, 3}}},
    {5, {{20.0, 505.0, 5, 2, 0}}},
    {6, {}},
    {7, {{98.75, 501.25, 7, 3, 0}}},
    {8, {}},
    {9, {}},
    {10, {{100.0, 500.0, 40, 4, 0}}},
    {11, {{300.0, 300.0, 46, 5, 0}}},
    {12, {{300.0, 300.0, 47, 5, 1}, {205.0, 395.0, 47, 6, 0}}},
    {13, {{205.0, 395.0, 48, 6, 1}, {100.0, 500.0, 48, 7, 0}}},
    {14, {{105.0, 495.0, 49, 7, 1}}},
};

std::vector<int> molecule_positions = { 1, 0, 2, 3, 4, 5, 6, 7 };

void AnnounceTestData(output::Output& target) {
    input::Traits<Localization> metadata;
    metadata.position_x().is_given = true;
    metadata.position_x().range().first = 0.0f * si::meter;
    metadata.position_x().range().second = 6930E-9f * si::meter;
    metadata.position_uncertainty_x().is_given = true;
    metadata.position_y().is_given = true;
    metadata.position_y().range().first = 0.0f * si::meter;
    metadata.position_y().range().second = 1260E-9f * si::meter;
    metadata.position_uncertainty_y().is_given = true;
    metadata.image_number().is_given = true;
    metadata.image_number().range().first = 0 * camera::frame;
    metadata.image_number().range().second = 14 * camera::frame;
    input::Traits<output::LocalizedImage> traits(metadata);
    traits.group_field = input::GroupFieldSemantic::ImageNumber;
    target.announceStormSize(traits);
}

void PushTestData(output::Output& target) {
    for (const auto& image_data : testdata) {
        output::LocalizedImage image(image_data.first * camera::frame);
        for (const auto& localization_data : image_data.second) {
            Localization localization;
            localization.position_x() = localization_data.x * 1E-9f * si::meter;
            localization.position_y() = localization_data.y * 1E-9f * si::meter;
            localization.position_uncertainty_x() = 20E-9f * si::meter;
            localization.position_uncertainty_y() = 20E-9f * si::meter;
            localization.frame_number() = int(localization_data.t) * camera::frame;
            image.push_back(localization);
        }

        target.receiveLocalizations(image);
    }
}

void TestLinking() {
    FakeOutput* results = new FakeOutput();
    Config config;
    config.allowBlinking = 1 * camera::frame;
    auto output = create_default(config, std::unique_ptr<output::Output>(results));
    AnnounceTestData(*output);
    PushTestData(*output);
    output->store_results(true);

    BOOST_CHECK_EQUAL(static_cast<int>(input::GroupFieldSemantic::Molecule),
                      static_cast<int>(results->announcement->group_field));
    BOOST_CHECK_EQUAL(14,
                      results->announcement->image_number().range().second->value());
    BOOST_CHECK_EQUAL(8, results->results.size());
    for (const auto& test_frame : testdata) {
        for (const auto& l : test_frame.second) {
            const auto& localized_image =
                results->results[molecule_positions[l.molecule]];
            const Localization& output = localized_image[l.offset_in_molecule];
            BOOST_CHECK_EQUAL(l.x * 1E-9f, output.position_x().value());
            BOOST_CHECK_EQUAL(l.y * 1E-9f, output.position_y().value());
            BOOST_CHECK_EQUAL(l.t, output.frame_number().value());
            BOOST_CHECK_EQUAL(l.molecule, localized_image.group);
            BOOST_CHECK_EQUAL(l.molecule, output.molecule().value());
        }
    }
}

boost::unit_test::test_suite* test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "EmissionTracker" );
    rv->add(BOOST_TEST_CASE(&TestLinking));
    return rv;
}

}
}
}
