#include "TerrariumElevationDataDecoder.h"

namespace carto
{

    TerrariumElevationDataDecoder::TerrariumElevationDataDecoder() :
        ElevationDecoder()
    {
    }

    TerrariumElevationDataDecoder::~TerrariumElevationDataDecoder()
    {
    }

    std::array<double, 4> TerrariumElevationDataDecoder::getColorComponentCoefficients() const
    {
        return COMPONENTS;
    }

    std::array<float, 4> TerrariumElevationDataDecoder::getVectorTileScales() const
    {
        return SCALES;
    }

    const std::array<double, 4> TerrariumElevationDataDecoder::COMPONENTS = std::array<double, 4>{256.0f, 1.0f, 1.0f / 256, -32768.0f};
    const std::array<float, 4> TerrariumElevationDataDecoder::SCALES = std::array<float, 4>{256.0f, 1.0f, 1.0f / 256, -32768.0f};
} // namespace carto
