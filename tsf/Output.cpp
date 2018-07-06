#include <boost/math/constants/constants.hpp>

#include "output/OutputSource.h"

#include <endian.h>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "output/FileOutputBuilder.h"
#include "tsf/TSFProto.pb.h"

namespace dStorm {
namespace tsf {
namespace {

class OutputConfig {
 public:
  output::BasenameAdjustedFileEntry outputFile;

  static std::string get_name() { return "TSFFile"; }
  static std::string get_description() { return "TSF file"; }
  static simparm::UserLevel get_user_level() { return simparm::Beginner; }

  void attach_ui( simparm::NodeHandle at ) {
      outputFile.attach_ui( at );
  }

  OutputConfig() : outputFile("ToFile", "Write TSF file to", ".tsf") {}
};

class Output : public output::Output {
 public:
  Output(const OutputConfig& config)
      : filename(config.outputFile()), file_descriptor(-1), molecule(0) {}
  void announceStormSize(const Announcement &a) OVERRIDE;
  RunRequirements announce_run(const RunAnnouncement&) OVERRIDE;
  void receiveLocalizations(const EngineResult&) OVERRIDE;
  void store_results_(bool) OVERRIDE;

  void check_for_duplicate_filenames
          (std::set<std::string>& present_filenames) { 
      insert_filename_with_check( filename, present_filenames ); 
  }

 private:
  void BuildSpot(const Localization& localization, TSF::Spot* spot);
  void CloseFileIfOpen();

  const std::string filename;
  int file_descriptor;
  std::auto_ptr<google::protobuf::io::ZeroCopyOutputStream> file;
  input::Traits<Localization> traits;
  simparm::NodeHandle current_ui;
  int molecule;
};

void Output::CloseFileIfOpen() {
  if (file_descriptor != -1) {
    file.reset(NULL);
    close(file_descriptor);
    file_descriptor = -1;
  }
}

void Output::announceStormSize(const Announcement &a) {
  traits = a;
}

Output::RunRequirements Output::announce_run(const RunAnnouncement&) {
  CloseFileIfOpen();
  file_descriptor = open(filename.c_str(), O_WRONLY | O_TRUNC | O_CREAT,
                                           00666);
  file.reset(new google::protobuf::io::FileOutputStream(file_descriptor));
  google::protobuf::io::CodedOutputStream coded_output(file.get());
  coded_output.WriteRaw("\0\0\0\0"           // magic version bytes
                        "\0\0\0\0\0\0\0\0",  // placeholder for SpotList offset
                        12);
  return Output::RunRequirements();
}

void Output::BuildSpot(const Localization& localization, TSF::Spot* spot) {
  spot->set_molecule(this->molecule++);
  spot->set_channel(1);
  spot->set_frame(localization.frame_number() / camera::frame + 1);

  spot->set_x(localization.position_x() / (1E-9 * si::metre));
  spot->set_y(localization.position_y() / (1E-9 * si::metre));
  if (traits.position_z().is_given) {
    spot->set_z(localization.position_z() / (1E-9 * si::metre));
  }

  if (traits.position_uncertainty_x().is_given) {
    spot->set_x_precision(localization.position_uncertainty_x() / (1E-9 * si::metre));
  }
  if (traits.position_uncertainty_y().is_given) {
    spot->set_y_precision(localization.position_uncertainty_y() / (1E-9 * si::metre));
  }
  if (traits.position_uncertainty_z().is_given) {
    spot->set_z_precision(localization.position_uncertainty_z() / (1E-9 * si::metre));
  }

  if (traits.amplitude().is_given) {
    spot->set_intensity(localization.amplitude() / camera::ad_count);
  }

  if (traits.local_background().is_given) {
    spot->set_background(localization.amplitude() / camera::ad_count);
  }

  if ((traits.psf_width_x().is_given || traits.psf_width_x().static_value) &&
      (traits.psf_width_y().is_given || traits.psf_width_y().static_value)) {
    double wx, wy;
    if (traits.psf_width_x().static_value) {
      wx = *traits.psf_width_x().static_value / (1E-9 * si::metre);
    } else {
      wx = localization.psf_width_x() / (1E-9 * si::metre);
    }

    if (traits.psf_width_y().static_value) {
      wy = *traits.psf_width_y().static_value / (1E-9 * si::metre);
    } else {
      wy = localization.psf_width_y() / (1E-9 * si::metre);
    }

    spot->set_width(sqrt(wx * wy));
    if (wx >= wy) {
      spot->set_a(wx / wy);
      spot->set_theta(0);
    } else {
      spot->set_a(wy / wx);
      spot->set_theta(0.5 * boost::math::constants::pi<double>());
    }
  }
}

void Output::receiveLocalizations(const EngineResult& er) {
  for (const Localization& localization : er) {
    TSF::Spot spot;
    BuildSpot(localization, &spot);
    {
      google::protobuf::io::CodedOutputStream coded_output(file.get());
      coded_output.WriteVarint32(spot.ByteSize());
    }
    spot.SerializeToZeroCopyStream(file.get());
  }
}

void Output::store_results_(bool) {
  int64_t spot_list_offset = file->ByteCount() - 12;
  TSF::SpotList spot_list;
  // ID was allocated by Nico Stuurman on 2014-03-07 for rapidSTORM
  spot_list.set_application_id(3);
  {
    google::protobuf::io::CodedOutputStream coded_output(file.get());
    coded_output.WriteVarint32(spot_list.ByteSize());
  }
  spot_list.SerializeToZeroCopyStream(file.get());
  file.reset();

  lseek(file_descriptor, 4, SEEK_SET);
  int64_t big_endian_offset = htobe64(spot_list_offset);
  write(file_descriptor, &big_endian_offset, 8);

  CloseFileIfOpen();
}

}  // namespace

std::auto_ptr<output::OutputSource> CreateOutput() {
    return std::auto_ptr<output::OutputSource>( new output::FileOutputBuilder<OutputConfig,Output>() );
}

}
}
