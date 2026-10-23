-- Base auxiliar para los tests de integración (make itest).
CREATE DATABASE IF NOT EXISTS hubflow_test CHARACTER SET utf8mb4;
GRANT ALL PRIVILEGES ON hubflow_test.* TO 'hubflow'@'%';
FLUSH PRIVILEGES;
