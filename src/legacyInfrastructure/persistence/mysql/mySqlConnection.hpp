#ifndef HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_CONNECTION_HPP
#define HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_CONNECTION_HPP

#include "infrastructure/config/settings.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

struct MYSQL;

namespace hubflow::infrastructure
{

/// @brief Valor de un parámetro de sentencia preparada.
using SqlValue = std::variant<std::nullptr_t, std::string, std::int64_t, std::uint64_t, double>;

/// @brief Fila de resultado: cada columna como texto (o vacío si es NULL).
using SqlRow = std::vector<std::optional<std::string>>;

/// @brief Error de acceso a la base de datos.
class database_error final : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/**
 * @brief Conexión a MySQL que ejecuta exclusivamente sentencias preparadas (sin concatenar
 * datos de usuario en SQL). Reintenta la conexión al iniciar y reconecta si el servidor la cerró.
 *
 * No es thread-safe: el servidor HTTP serializa el acceso.
 */
class MySqlConnection final
{
public:
    /// @throws database_error si no logra conectarse tras los reintentos configurados.
    explicit MySqlConnection(DatabaseSettings settings);

    ~MySqlConnection();

    MySqlConnection(const MySqlConnection&)            = delete;
    MySqlConnection& operator=(const MySqlConnection&) = delete;

    /// @brief Ejecuta una sentencia sin resultado. @return filas afectadas.
    std::uint64_t execute(const std::string& sql, const std::vector<SqlValue>& parameters = {});

    /// @brief Ejecuta una consulta y devuelve todas sus filas.
    std::vector<SqlRow> query(const std::string& sql, const std::vector<SqlValue>& parameters = {});

    /// @brief Ejecuta un script con varias sentencias separadas por ';' (sin parámetros).
    void executeScript(const std::string& script);

private:
    struct HandleCloser
    {
        void operator()(MYSQL* pHandle) const noexcept;
    };

    void connect();
    void ensureConnected();

    DatabaseSettings                     m_settings;
    std::unique_ptr<MYSQL, HandleCloser> m_spHandle;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_CONNECTION_HPP
