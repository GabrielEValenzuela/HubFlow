#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_DIMENSIONS_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_DIMENSIONS_HPP

namespace hubflow::logistics::domain
{

    /**
     * @brief Medidas de un bulto en centímetros: largo, ancho y alto, cada una mayor a 0 y como máximo 300.
     *
     * Con ellas se calcula el peso volumétrico, que es lo que ocupa el bulto en el vehículo expresado en kilos:
     * largo × ancho × alto dividido por 5000.
     */
    class Dimensions final
    {
    public:
        static constexpr double MAX_CENTIMETERS {300.0};
        static constexpr double VOLUMETRIC_DIVISOR {5000.0};

        /// @throws BusinessError (Validation) si alguna medida está fuera de rango.
        Dimensions(double lengthCm, double widthCm, double heightCm);

        double lengthCm() const noexcept;
        double widthCm() const noexcept;
        double heightCm() const noexcept;

        /// @brief Peso volumétrico en kilogramos.
        double volumetricWeightKg() const noexcept;

    private:
        double m_lengthCm;
        double m_widthCm;
        double m_heightCm;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_DIMENSIONS_HPP
