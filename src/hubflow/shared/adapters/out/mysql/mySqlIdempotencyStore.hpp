#ifndef HUBFLOW_SHARED_ADAPTERS_OUT_MYSQL_MY_SQL_IDEMPOTENCY_STORE_HPP
#define HUBFLOW_SHARED_ADAPTERS_OUT_MYSQL_MY_SQL_IDEMPOTENCY_STORE_HPP

#include "hubflow/platform/mysql/mySqlConnection.hpp"
#include "hubflow/shared/application/ports/out/idempotencyStore.hpp"

namespace hubflow::shared::adapters
{

/**
 * @brief Respuestas ya entregadas, guardadas en la tabla @c idempotency_keys.
 *
 * Si la misma clave llega dos veces para el mismo usuario, la segunda vez conserva la primera respuesta.
 */
class MySqlIdempotencyStore final : public application::IdempotencyStore
{
public:
    explicit MySqlIdempotencyStore(platform::MySqlConnection& connection);

private:
    std::optional<application::StoredResponse> find(std::uint64_t userId, const std::string& key) const override;
    void save(std::uint64_t userId, const std::string& key, const application::StoredResponse& response) override;

    platform::MySqlConnection& m_connection;
};

} // namespace hubflow::shared::adapters

#endif // HUBFLOW_SHARED_ADAPTERS_OUT_MYSQL_MY_SQL_IDEMPOTENCY_STORE_HPP
