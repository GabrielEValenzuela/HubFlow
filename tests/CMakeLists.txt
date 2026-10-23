include(GoogleTest)

add_library(hubflow_test_support STATIC
    support/inMemoryRepositories.cpp
    support/fixtures.cpp
    support/applicationHarness.cpp)
target_include_directories(hubflow_test_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(hubflow_test_support PUBLIC hubflow_application)

file(GLOB_RECURSE HUBFLOW_UNIT_TESTS CONFIGURE_DEPENDS unit/*.cpp application/*.cpp adapters/*.cpp)
add_executable(hubflow_tests ${HUBFLOW_UNIT_TESTS})
target_link_libraries(hubflow_tests PRIVATE hubflow_test_support hubflow_export GTest::gtest_main)
gtest_discover_tests(hubflow_tests DISCOVERY_TIMEOUT 30)

if(HUBFLOW_BUILD_SERVER)
    file(GLOB_RECURSE HUBFLOW_SERVER_TESTS CONFIGURE_DEPENDS http/*.cpp)
    if(HUBFLOW_SERVER_TESTS AND TARGET hubflow_infrastructure)
        add_executable(hubflow_server_tests ${HUBFLOW_SERVER_TESTS})
        target_link_libraries(hubflow_server_tests PRIVATE hubflow_infrastructure hubflow_test_support GTest::gtest_main)
        gtest_discover_tests(hubflow_server_tests DISCOVERY_TIMEOUT 30)
    endif()

    file(GLOB_RECURSE HUBFLOW_INTEGRATION_TESTS CONFIGURE_DEPENDS integration/*.cpp)
    if(HUBFLOW_INTEGRATION_TESTS AND TARGET hubflow_infrastructure)
        add_executable(hubflow_integration_tests ${HUBFLOW_INTEGRATION_TESTS})
        target_link_libraries(hubflow_integration_tests PRIVATE hubflow_infrastructure hubflow_test_support GTest::gtest_main)
        install(TARGETS hubflow_integration_tests RUNTIME DESTINATION bin)
    endif()
endif()
