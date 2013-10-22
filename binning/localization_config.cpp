#include "localization_config_impl.h"
#include "dStorm/Localization.h"

namespace dStorm {
namespace binning {

template <> std::string get_label<dStorm::Localization::Fields::PositionX>() { return "PositionX"; }
template <> std::string get_label<dStorm::Localization::Fields::PositionY>() { return "PositionY"; }
template <> std::string get_label<dStorm::Localization::Fields::PositionZ>() { return "PositionZ"; }
template <> std::string get_label<dStorm::Localization::Fields::PositionUncertaintyX>() { return "PositionUncertaintyX"; }
template <> std::string get_label<dStorm::Localization::Fields::PositionUncertaintyY>() { return "PositionUncertaintyY"; }
template <> std::string get_label<dStorm::Localization::Fields::PositionUncertaintyZ>() { return "PositionUncertaintyZ"; }
template <> std::string get_label<dStorm::Localization::Fields::PSFWidthX>() { return "PSFWidthX"; }
template <> std::string get_label<dStorm::Localization::Fields::PSFWidthY>() { return "PSFWidthY"; }
template <> std::string get_label<dStorm::Localization::Fields::ImageNumber>() { return "ImageNumber"; }
template <> std::string get_label<dStorm::Localization::Fields::Amplitude>() { return "Amplitude"; }
template <> std::string get_label<dStorm::Localization::Fields::TwoKernelImprovement>() { return "TwoKernelImprovement"; }
template <> std::string get_label<dStorm::Localization::Fields::FitResidues>() { return "FitResidues"; }
template <> std::string get_label<dStorm::Localization::Fields::Fluorophore>() { return "Fluorophore"; }
template <> std::string get_label<dStorm::Localization::Fields::LocalBackground>() { return "LocalBackground"; }

#define INSTANTIATE_WITH_LOCALIZATION_FIELD_INDEX(x) \
    template class LocalizationConfig<x>;
#include <dStorm/localization/expand.h>

}
}
