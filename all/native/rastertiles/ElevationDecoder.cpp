#include "ElevationDecoder.h"
#include "utils/Log.h"
#include "utils/TileUtils.h"

#include <algorithm>

namespace carto
{
    ElevationDecoder::~ElevationDecoder()
    {
    }

    ElevationDecoder::ElevationDecoder()
    {
    }
    
    double ElevationDecoder::decodeHeight(const Color &encodedHeight) const
    {
        // the 4th component is used to "translate" the result
        const std::array<float, 4> components = getColorComponentCoefficients();
        return (double)(components[0] * encodedHeight.getR() + components[1] * encodedHeight.getG() + components[2] * encodedHeight.getB() + components[3] * encodedHeight.getA());
    }
} // namespace carto
