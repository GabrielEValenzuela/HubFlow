#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_WEIGHT_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_WEIGHT_HPP

namespace hubflow::logistics::domain
{

    /// @brief Peso del paquete en kilogramos, en el intervalo (0, 50].
    class Weight final
    {
    public:
        static constexpr double MAX_KILOGRAMS {50.0};

        /// @throws BusinessError (Validation) si está fuera de rango.
        explicit Weight(double kilograms);

        double kilograms() const noexcept;

    private:
        double m_kilograms;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_WEIGHT_HPP
