include(FetchContent)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
set(FETCHCONTENT_QUIET ON)

if(HUBFLOW_BUILD_TESTS)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

if(HUBFLOW_BUILD_TESTS OR HUBFLOW_BUILD_SERVER)
    FetchContent_Declare(nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(nlohmann_json)

    FetchContent_Declare(httplib
        URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.15.3.tar.gz
        URL_HASH SHA256=2121bbf38871bb2aafb5f7f2b9b94705366170909f434428352187cb0216124e
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(httplib)
    get_target_property(HUBFLOW_HTTPLIB_INCLUDES httplib INTERFACE_INCLUDE_DIRECTORIES)
    set_target_properties(httplib PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${HUBFLOW_HTTPLIB_INCLUDES}")
endif()

function(hubflow_require_server_dependencies)
    find_package(OpenSSL REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MYSQLCLIENT REQUIRED IMPORTED_TARGET mysqlclient)
    pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)
    add_library(hubflow_mysql INTERFACE)
    target_link_libraries(hubflow_mysql INTERFACE PkgConfig::MYSQLCLIENT)
    add_library(hubflow_sodium INTERFACE)
    target_link_libraries(hubflow_sodium INTERFACE PkgConfig::SODIUM)
endfunction()
