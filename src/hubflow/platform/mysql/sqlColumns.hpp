#ifndef HUBFLOW_PLATFORM_MYSQL_SQL_COLUMNS_HPP
#define HUBFLOW_PLATFORM_MYSQL_SQL_COLUMNS_HPP

#include "hubflow/platform/mysql/mySqlConnection.hpp"

#include <optional>
#include <string>

namespace hubflow::platform
{

/// @brief Lectura tipada de columnas de un SqlRow.
class SqlColumns final
{
public:
    static std::string   text(const SqlRow& row, std::size_t column);
    static std::int64_t  signedNumber(const SqlRow& row, std::size_t column);
    static std::uint64_t unsignedNumber(const SqlRow& row, std::size_t column);
    static double        decimal(const SqlRow& row, std::size_t column);
    static bool          flag(const SqlRow& row, std::size_t column);

    /**
     * @brief Traduce un nombre del vocabulario a su enumerado.
     * @throws database_error si el valor almacenado no es válido.
     */
    template <typename TEnum>
    static TEnum term(const std::optional<TEnum>& parsed, const std::string& column)
    {
        if (!parsed.has_value())
        {
            throw database_error {"Valor inválido en la columna " + column};
        }
        return *parsed;
    }
};

} // namespace hubflow::platform

#endif // HUBFLOW_PLATFORM_MYSQL_SQL_COLUMNS_HPP
