SET NAMES utf8mb4;
-- Respuestas ya entregadas para reintentos seguros de operaciones no idempotentes.

CREATE TABLE IF NOT EXISTS idempotency_keys (
    user_id    BIGINT UNSIGNED   NOT NULL,
    idem_key   VARCHAR(80)       NOT NULL,
    method     VARCHAR(8)        NOT NULL,
    path       VARCHAR(255)      NOT NULL,
    status     SMALLINT UNSIGNED NOT NULL,
    body       MEDIUMTEXT        NOT NULL,
    created_at TIMESTAMP         NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (user_id, idem_key)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;
