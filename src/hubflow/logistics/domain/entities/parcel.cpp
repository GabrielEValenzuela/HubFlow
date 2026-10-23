#include "hubflow/logistics/domain/entities/parcel.hpp"

namespace hubflow::logistics::domain
{

Parcel::Parcel(const std::string& trackingCode,
               const std::size_t  ordinal,
               const std::size_t  total,
               const ParcelData&  data)
    : m_label {trackingCode + "-" + std::to_string(ordinal) + "/" + std::to_string(total)}
    , m_ordinal {ordinal}
    , m_weight {data.weightKg}
    , m_dimensions {data.lengthCm, data.widthCm, data.heightCm}
    , m_delivered {data.delivered}
{
}

void Parcel::confirmDelivery() noexcept
{
    m_delivered = true;
}

const std::string& Parcel::label() const noexcept
{
    return m_label;
}

std::size_t Parcel::ordinal() const noexcept
{
    return m_ordinal;
}

double Parcel::weightKg() const noexcept
{
    return m_weight.kilograms();
}

const Dimensions& Parcel::dimensions() const noexcept
{
    return m_dimensions;
}

double Parcel::volumetricWeightKg() const noexcept
{
    return m_dimensions.volumetricWeightKg();
}

double Parcel::billableWeightKg() const noexcept
{
    return weightKg() > volumetricWeightKg() ? weightKg() : volumetricWeightKg();
}

bool Parcel::delivered() const noexcept
{
    return m_delivered;
}

ParcelData Parcel::data() const noexcept
{
    return ParcelData {
        weightKg(), m_dimensions.lengthCm(), m_dimensions.widthCm(), m_dimensions.heightCm(), m_delivered};
}

} // namespace hubflow::logistics::domain
