#include "hubflow/logistics/domain/valueobjects/dimensions.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{
    namespace
    {
        bool isMeasure(const double centimeters)
        {
            return centimeters > 0.0 && centimeters <= Dimensions::MAX_CENTIMETERS;
        }
    } // namespace

    Dimensions::Dimensions(const double lengthCm, const double widthCm, const double heightCm)
        : m_lengthCm {lengthCm}
        , m_widthCm {widthCm}
        , m_heightCm {heightCm}
    {
        if (!isMeasure(lengthCm) || !isMeasure(widthCm) || !isMeasure(heightCm))
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                  "dimensions.invalid",
                                                  "Largo, ancho y alto deben ser mayores a 0 y como máximo 300 cm"};
        }
    }

    double Dimensions::lengthCm() const noexcept
    {
        return m_lengthCm;
    }

    double Dimensions::widthCm() const noexcept
    {
        return m_widthCm;
    }

    double Dimensions::heightCm() const noexcept
    {
        return m_heightCm;
    }

    double Dimensions::volumetricWeightKg() const noexcept
    {
        return m_lengthCm * m_widthCm * m_heightCm / VOLUMETRIC_DIVISOR;
    }

} // namespace hubflow::logistics::domain
