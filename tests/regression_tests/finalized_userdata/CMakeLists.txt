add_executable(sol2.tests.finalized_userdata main.cpp)
target_link_libraries(sol2.tests.finalized_userdata PRIVATE sol2::sol2 Lua::Lua ${CMAKE_DL_LIBS})
target_compile_definitions(sol2.tests.finalized_userdata PRIVATE SOL_ALL_SAFETIES_ON=1)
add_test(NAME sol2.tests.finalized_userdata COMMAND sol2.tests.finalized_userdata)
