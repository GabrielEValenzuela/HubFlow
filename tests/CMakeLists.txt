include(GoogleTest)

add_library(hubflow_test_support STATIC
    support/inMemoryRepositories.cpp
    support/fixtures.cpp
    support/applicationHarness.cpp)
add_library(hubflow_http_test_support STATIC support/httpHarness.cpp)
target_link_libraries(hubflow_http_test_support PUBLIC hubflow_test_support hubflow_http)
target_include_directories(hubflow_test_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(hubflow_test_support PUBLIC hubflow_application)

file(GLOB_RECURSE HUBFLOW_UNIT_TESTS CONFIGURE_DEPENDS unit/*.cpp application/*.cpp adapters/*.cpp)
add_executable(hubflow_tests ${HUBFLOW_UNIT_TESTS})
target_link_libraries(hubflow_tests PRIVATE hubflow_test_support hubflow_export GTest::gtest_main)
gtest_discover_tests(hubflow_tests DISCOVERY_TIMEOUT 30)

file(GLOB_RECURSE HUBFLOW_HTTP_TESTS CONFIGURE_DEPENDS http/*.cpp)
add_executable(hubflow_http_tests ${HUBFLOW_HTTP_TESTS})
target_link_libraries(hubflow_http_tests PRIVATE hubflow_http_test_support GTest::gtest_main)
gtest_discover_tests(hubflow_http_tests DISCOVERY_TIMEOUT 30)

if(HUBFLOW_BUILD_SERVER)
    file(GLOB_RECURSE HUBFLOW_INTEGRATION_TESTS CONFIGURE_DEPENDS integration/*.cpp)
    add_executable(hubflow_integration_tests ${HUBFLOW_INTEGRATION_TESTS})
    target_compile_definitions(hubflow_integration_tests PRIVATE HUBFLOW_MIGRATIONS_DIR="${PROJECT_SOURCE_DIR}/db/migrations")
    target_link_libraries(hubflow_integration_tests PRIVATE hubflow_adapters hubflow_test_support GTest::gtest_main)
    gtest_discover_tests(hubflow_integration_tests TEST_PREFIX Integration. DISCOVERY_TIMEOUT 30)
endif()
