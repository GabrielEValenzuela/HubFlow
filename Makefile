SHELL := /bin/bash
COMPOSE := docker compose
HTTP_PORT = $(shell sed -n 's/^HUBFLOW_HTTP_PORT=//p' .env 2>/dev/null | tail -1 | grep . || echo 8480)

.DEFAULT_GOAL := help
.PHONY: help env up down logs ps reset-db build test asan-test itest format format-check tidy tidy-fix cppcheck lint docs clean

help: ## Lista los targets disponibles
	@grep -E '^[a-z-]+:.*## ' $(MAKEFILE_LIST) | awk -F ':.*## ' '{printf "  %-14s %s\n", $$1, $$2}'

env:
	@if [ ! -f .env ]; then \
	  cp .env.example .env; \
	  secret=$$(openssl rand -base64 48 | tr -dc 'A-Za-z0-9' | head -c 48); \
	  sed -i "s|^HUBFLOW_JWT_SECRET=.*|HUBFLOW_JWT_SECRET=$$secret|" .env; \
	  echo "[hubflow] .env creado con un HUBFLOW_JWT_SECRET aleatorio"; \
	fi

up: env ## Construye y levanta db + api, espera la salud y abre el dashboard
	$(COMPOSE) up -d --build
	@port=$(HTTP_PORT); echo "[hubflow] esperando http://localhost:$$port/api/v1/health ..."; \
	for i in $$(seq 1 90); do \
	  if curl -fsS "http://localhost:$$port/api/v1/health" >/dev/null 2>&1; then ok=1; break; fi; sleep 2; \
	done; \
	if [ -z "$$ok" ]; then echo "[hubflow] la API no respondio en 180 s"; $(COMPOSE) logs --tail=40 api; exit 1; fi; \
	echo "[hubflow] listo: http://localhost:$$port"; \
	echo "[hubflow] usuarios: admin@, operador@, pum1@, pum2@hubflow.local (clave: HUBFLOW_SEED_PASSWORD de .env, por defecto hubflow123)"; \
	if command -v xdg-open >/dev/null 2>&1; then xdg-open "http://localhost:$$port" >/dev/null 2>&1 || true; fi

down: ## Detiene el stack (conserva los datos)
	$(COMPOSE) down

logs: ## Sigue los logs del stack
	$(COMPOSE) logs -f --tail=100

ps: ## Estado de los contenedores del proyecto
	$(COMPOSE) ps

reset-db: env ## Borra el volumen hubflow-db (pierde los datos)
	$(COMPOSE) down -v

build: ## Compila en el host (preset debug)
	cmake --preset debug
	cmake --build --preset debug

test: build ## Corre los tests en el host
	ctest --preset debug

asan-test: ## Tests con AddressSanitizer + UBSan
	cmake --preset asan
	cmake --build --preset asan
	ctest --preset asan

itest: env ## Tests de integracion contra MySQL (hubflow_test) dentro del contenedor
	$(COMPOSE) up -d --build db api
	$(COMPOSE) run --rm --no-deps -e DB_TEST_NAME=hubflow_test api hubflow_integration_tests

format: ## Aplica clang-format
	@scripts/format.sh

format-check: ## Verifica clang-format sin modificar
	@scripts/format.sh --check

tidy: ## Analisis estatico con clang-tidy
	@scripts/tidy.sh

tidy-fix: ## clang-tidy aplicando correcciones automaticas
	@scripts/tidy.sh --fix

cppcheck: ## Analisis estatico con cppcheck
	@scripts/cppcheck.sh

lint: ## format-check + clang-tidy + cppcheck con resumen
	@scripts/lint.sh

docs: ## Genera la documentacion Doxygen en build/docs
	doxygen Doxyfile

clean: ## Borra build/
	rm -rf build
