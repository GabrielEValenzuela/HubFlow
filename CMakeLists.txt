cmake_minimum_required(VERSION 3.20)

project(hubflow
    VERSION 1.0.0
    DESCRIPTION "HubFlow - gestión de envíos de última milla"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(HUBFLOW_BUILD_TESTS "Compila los tests con GoogleTest" ON)
option(HUBFLOW_BUILD_SERVER "Compila el servidor REST (requiere MySQL client, libsodium y OpenSSL)" OFF)
option(HUBFLOW_WARNINGS_AS_ERRORS "Trata los warnings como errores" ON)
option(HUBFLOW_SANITIZE "Compila con AddressSanitizer y UndefinedBehaviorSanitizer" OFF)

include(cmake/compilerOptions.cmake)
include(cmake/dependencies.cmake)

file(GLOB_RECURSE HUBFLOW_DOMAIN_SOURCES CONFIGURE_DEPENDS
    src/hubflow/*/domain/*.cpp)
add_library(hubflow_domain STATIC ${HUBFLOW_DOMAIN_SOURCES})
target_include_directories(hubflow_domain PUBLIC src)
target_link_libraries(hubflow_domain PUBLIC hubflow_options)

file(GLOB_RECURSE HUBFLOW_APPLICATION_SOURCES CONFIGURE_DEPENDS
    src/hubflow/*/application/*.cpp
    src/hubflow/logistics/adapters/out/routing/*.cpp)
add_library(hubflow_application STATIC ${HUBFLOW_APPLICATION_SOURCES})
target_link_libraries(hubflow_application PUBLIC hubflow_domain)

if(HUBFLOW_BUILD_SERVER)
    hubflow_require_server_dependencies()
    file(GLOB_RECURSE HUBFLOW_INFRASTRUCTURE_SOURCES CONFIGURE_DEPENDS src/legacyInfrastructure/*.cpp)
    add_library(hubflow_infrastructure STATIC ${HUBFLOW_INFRASTRUCTURE_SOURCES})
    target_link_libraries(hubflow_infrastructure
        PUBLIC hubflow_application httplib::httplib nlohmann_json::nlohmann_json
        PRIVATE hubflow_mysql hubflow_sodium OpenSSL::Crypto)

    add_executable(hubflow src/main.cpp)
    target_link_libraries(hubflow PRIVATE hubflow_infrastructure)
    install(TARGETS hubflow RUNTIME DESTINATION bin)
endif()

if(HUBFLOW_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
