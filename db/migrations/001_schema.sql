-- Esquema de HubFlow. MySQL es la fuente de verdad durable; las consultas del negocio se
-- resuelven con las estructuras de datos propias en memoria, hidratadas al iniciar.

CREATE TABLE IF NOT EXISTS users (
    id            BIGINT UNSIGNED  NOT NULL PRIMARY KEY,
    email         VARCHAR(120)     NOT NULL UNIQUE,
    name          VARCHAR(80)      NOT NULL,
    role          ENUM('ADMIN', 'OPERADOR', 'PUM') NOT NULL,
    password_hash VARCHAR(255)     NOT NULL,
    active        TINYINT(1)       NOT NULL DEFAULT 1,
    created_at    TIMESTAMP        NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS road_nodes (
    id       VARCHAR(16)  NOT NULL PRIMARY KEY,
    position INT UNSIGNED NOT NULL UNIQUE,
    label    VARCHAR(80)  NOT NULL,
    is_hub   TINYINT(1)   NOT NULL DEFAULT 0,
    zone     ENUM('CENTRO', 'NORTE', 'SUR') NOT NULL,
    x        DOUBLE       NOT NULL,
    y        DOUBLE       NOT NULL
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS road_edges (
    id      INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    from_id VARCHAR(16)  NOT NULL,
    to_id   VARCHAR(16)  NOT NULL,
    cost_km DECIMAL(6, 2) NOT NULL,
    CONSTRAINT fk_edge_from FOREIGN KEY (from_id) REFERENCES road_nodes (id),
    CONSTRAINT fk_edge_to FOREIGN KEY (to_id) REFERENCES road_nodes (id),
    CONSTRAINT uq_edge UNIQUE (from_id, to_id)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS shipments (
    code           CHAR(8)          NOT NULL PRIMARY KEY,
    recipient      VARCHAR(80)      NOT NULL,
    zone           ENUM('CENTRO', 'NORTE', 'SUR') NOT NULL,
    weight_kg      DECIMAL(5, 2)    NOT NULL,
    service        ENUM('EXPRESS', 'PRIORITARIO', 'ESTANDAR') NOT NULL,
    delivery_point VARCHAR(16)      NOT NULL,
    status         ENUM('RECIBIDO', 'CLASIFICADO', 'EN_REPARTO', 'REPROGRAMADO', 'ENTREGADO') NOT NULL,
    attempts       INT UNSIGNED     NOT NULL DEFAULT 0,
    state_bits     TINYINT UNSIGNED NOT NULL DEFAULT 0,
    queue_ticket   BIGINT UNSIGNED  NOT NULL,
    created_at     TIMESTAMP        NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at     TIMESTAMP        NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT fk_shipment_point FOREIGN KEY (delivery_point) REFERENCES road_nodes (id)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS movements (
    shipment_code CHAR(8)      NOT NULL,
    sequence      INT UNSIGNED NOT NULL,
    status        ENUM('RECIBIDO', 'CLASIFICADO', 'EN_REPARTO', 'REPROGRAMADO', 'ENTREGADO') NOT NULL,
    note          VARCHAR(200) NOT NULL,
    actor_name    VARCHAR(80)  NOT NULL,
    occurred_at   BIGINT       NOT NULL,
    PRIMARY KEY (shipment_code, sequence),
    CONSTRAINT fk_movement_shipment FOREIGN KEY (shipment_code) REFERENCES shipments (code)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS routes (
    id           BIGINT UNSIGNED NOT NULL PRIMARY KEY,
    courier_id   BIGINT UNSIGNED NOT NULL,
    courier_name VARCHAR(80)     NOT NULL,
    algorithm    ENUM('BFS', 'DFS', 'GREEDY', 'ASTAR') NOT NULL,
    created_at   BIGINT          NOT NULL,
    CONSTRAINT fk_route_courier FOREIGN KEY (courier_id) REFERENCES users (id)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS route_stops (
    route_id      BIGINT UNSIGNED NOT NULL,
    stop_order    INT UNSIGNED    NOT NULL,
    shipment_code CHAR(8)         NOT NULL,
    node_id       VARCHAR(16)     NOT NULL,
    outcome       ENUM('PENDIENTE', 'ENTREGADO', 'REPROGRAMADO') NOT NULL,
    PRIMARY KEY (route_id, stop_order),
    CONSTRAINT fk_stop_route FOREIGN KEY (route_id) REFERENCES routes (id),
    CONSTRAINT fk_stop_shipment FOREIGN KEY (shipment_code) REFERENCES shipments (code)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS audit_events (
    id          BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    occurred_at BIGINT          NOT NULL,
    actor_id    BIGINT UNSIGNED NOT NULL,
    actor_name  VARCHAR(80)     NOT NULL,
    type        VARCHAR(40)     NOT NULL,
    subject     VARCHAR(80)     NOT NULL,
    detail      VARCHAR(255)    NOT NULL
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;
