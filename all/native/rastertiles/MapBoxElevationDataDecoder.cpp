#include "MapBoxElevationDataDecoder.h"

namespace carto
{

    MapBoxElevationDataDecoder::MapBoxElevationDataDecoder() :
        ElevationDecoder()
    {
    }

    MapBoxElevationDataDecoder::~MapBoxElevationDataDecoder()
    {
    }

    std::array<float, 4> MapBoxElevationDataDecoder::getColorComponentCoefficients() const
    {
        return COMPONENTS;
    }

    std::array<float, 4> MapBoxElevationDataDecoder::getVectorTileScales() const
    {
        return SCALES;
    }

    const std::array<float, 4> MapBoxElevationDataDecoder::COMPONENTS = std::array<float, 4>{256 * 256 * 0.1f, 256 * 0.1f, 0.1f, -10000.0f};
    const std::array<float, 4> MapBoxElevationDataDecoder::SCALES = std::array<float, 4>{256 * 256, 256.0f, 1.0f, 0.0f};
} // namespace carto
