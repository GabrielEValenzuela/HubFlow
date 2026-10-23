SET NAMES utf8mb4;
-- Zonas, niveles de servicio, características y permisos por rol pasan a ser datos.
-- Agregar o desactivar uno de ellos es una operación de la API; no requiere tocar el programa.
-- Esta migración reemplaza las columnas ENUM de zona y servicio por referencias a estas tablas.
-- Docker sólo corre las migraciones sobre un volumen nuevo: para aplicarla sobre una base existente usar `make reset-db`.

CREATE TABLE IF NOT EXISTS zones (
    code     VARCHAR(16)  NOT NULL,
    position INT UNSIGNED NOT NULL AUTO_INCREMENT,
    name     VARCHAR(60)  NOT NULL,
    active   TINYINT(1)   NOT NULL DEFAULT 1,
    PRIMARY KEY (code),
    UNIQUE KEY uq_zone_position (position)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS service_levels (
    code     VARCHAR(16)  NOT NULL,
    position INT UNSIGNED NOT NULL AUTO_INCREMENT,
    name     VARCHAR(60)  NOT NULL,
    priority INT UNSIGNED NOT NULL,
    active   TINYINT(1)   NOT NULL DEFAULT 1,
    PRIMARY KEY (code),
    UNIQUE KEY uq_service_position (position)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS shipment_features (
    code     VARCHAR(16)  NOT NULL,
    position INT UNSIGNED NOT NULL AUTO_INCREMENT,
    name     VARCHAR(60)  NOT NULL,
    active   TINYINT(1)   NOT NULL DEFAULT 1,
    PRIMARY KEY (code),
    UNIQUE KEY uq_feature_position (position)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS role_permissions (
    role       VARCHAR(16) NOT NULL,
    permission VARCHAR(32) NOT NULL,
    PRIMARY KEY (role, permission)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

INSERT INTO zones (code, name) VALUES ('CENTRO', 'Centro'), ('NORTE', 'Norte'), ('SUR', 'Sur');

INSERT INTO service_levels (code, name, priority) VALUES
  ('EXPRESS', 'Express', 3), ('PRIORITARIO', 'Prioritario', 2), ('ESTANDAR', 'Estándar', 1);

INSERT INTO shipment_features (code, name) VALUES
  ('FRAGIL', 'Frágil'), ('REFRIGERADO', 'Refrigerado'), ('VOLUMINOSO', 'Voluminoso');

INSERT INTO role_permissions (role, permission) VALUES
  ('ADMIN', 'MANAGE_USERS'), ('ADMIN', 'OPERATE_SHIPMENTS'), ('ADMIN', 'OPERATE_SORTING_DESK'),
  ('ADMIN', 'ASSIGN_ROUTES'), ('ADMIN', 'RESOLVE_STOPS'), ('ADMIN', 'VIEW_INDEXES'), ('ADMIN', 'VIEW_AUDIT'),
  ('ADMIN', 'MANAGE_CATALOGS'),
  ('OPERADOR', 'OPERATE_SHIPMENTS'), ('OPERADOR', 'OPERATE_SORTING_DESK'), ('OPERADOR', 'ASSIGN_ROUTES'),
  ('OPERADOR', 'RESOLVE_STOPS'), ('OPERADOR', 'VIEW_INDEXES'),
  ('PUM', 'RESOLVE_STOPS');

ALTER TABLE road_nodes
    MODIFY zone VARCHAR(16) NOT NULL,
    ADD COLUMN active TINYINT(1) NOT NULL DEFAULT 1,
    ADD CONSTRAINT fk_node_zone FOREIGN KEY (zone) REFERENCES zones (code);

ALTER TABLE shipments
    MODIFY zone VARCHAR(16) NOT NULL,
    MODIFY service VARCHAR(16) NOT NULL,
    ADD CONSTRAINT fk_shipment_zone FOREIGN KEY (zone) REFERENCES zones (code),
    ADD CONSTRAINT fk_shipment_service FOREIGN KEY (service) REFERENCES service_levels (code);

CREATE TABLE IF NOT EXISTS shipment_feature_links (
    shipment_code CHAR(8)     NOT NULL,
    feature_code  VARCHAR(16) NOT NULL,
    PRIMARY KEY (shipment_code, feature_code),
    CONSTRAINT fk_link_shipment FOREIGN KEY (shipment_code) REFERENCES shipments (code),
    CONSTRAINT fk_link_feature FOREIGN KEY (feature_code) REFERENCES shipment_features (code)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;
