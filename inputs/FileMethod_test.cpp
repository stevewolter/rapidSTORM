#include "inputs/FileMethod.h"

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

#include "tiff/TIFF.h"
#include "test-plugin/DummyFileInput.h"
#include "simparm/text_stream/RootNode.h"

namespace dStorm {
namespace inputs {

class FileMethodTest {
  public:
    FileMethodTest() : io(new simparm::text_stream::RootNode()) {
        file_method.registerNamedEntries(io);
        file_method.add_choice( tiff::make_input() );
        file_method.publish_meta_info();
    }

    void set_input_file_name(boost::shared_ptr<simparm::text_stream::RootNode> io, std::string name) {
        std::stringstream command("in FileMethod in InputFile in value set " + name);
        io->processCommand(command);
    }

    void set_input_file_name(std::string name) {
        std::stringstream command("in FileMethod in InputFile in value set " + name);
        io->processCommand(command);
    }

    int size_y() {
        return size_y(file_method);
    }

    int size_y(const FileMethod& method) {
        return method.current_meta_info()->traits< dStorm::engine::ImageStack >()->plane(0).image.size[1].value();
    }

    boost::shared_ptr<simparm::text_stream::RootNode> io;
    FileMethod file_method;
};

BOOST_FIXTURE_TEST_CASE(PublishesEmptyInfoForEmptyFile, FileMethodTest) {
    BOOST_CHECK(file_method.current_meta_info()->provides_nothing());
}

BOOST_FIXTURE_TEST_CASE(PublishesInfo, FileMethodTest) {
    set_input_file_name(tiff::test_file_name);
    BOOST_CHECK_EQUAL(42, size_y());
}

BOOST_FIXTURE_TEST_CASE(StaysWithTIFFOnTwoInputs, FileMethodTest) {
    set_input_file_name(tiff::test_file_name);
    file_method.add_choice( dummy_file_input::make() );
    file_method.publish_meta_info();
    BOOST_CHECK_EQUAL(42, size_y());
}

BOOST_FIXTURE_TEST_CASE(SwitchesToDummyInput, FileMethodTest) {
    set_input_file_name(tiff::test_file_name);
    file_method.add_choice( dummy_file_input::make() );
    file_method.publish_meta_info();
    set_input_file_name("foobar.dummy");
    BOOST_CHECK_EQUAL(50, size_y());
}

BOOST_FIXTURE_TEST_CASE(CopiedMethodCanSwitchFiles, FileMethodTest) {
    set_input_file_name("foobar.dummy");
    FileMethod copy(file_method);
    auto copy_io = boost::make_shared<simparm::text_stream::RootNode>();
    copy.registerNamedEntries(copy_io);
    copy.publish_meta_info();
    set_input_file_name(copy_io, tiff::test_file_name);
    BOOST_CHECK_EQUAL(42, size_y(copy));
}

BOOST_FIXTURE_TEST_CASE(CopiedMethodIsIndependent, FileMethodTest) {
    file_method.publish_meta_info();
    set_input_file_name(tiff::test_file_name);
    FileMethod copy(file_method);
    auto copy_io = boost::make_shared<simparm::text_stream::RootNode>();
    copy.registerNamedEntries(copy_io);
    copy.publish_meta_info();
    set_input_file_name(copy_io, "");
    BOOST_CHECK_EQUAL(42, size_y());
}

}
}
