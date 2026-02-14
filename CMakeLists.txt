cmake_minimum_required(VERSION 3.10)
project(net_stats_tui LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(net-stats src/main.c)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(NCURSESW QUIET IMPORTED_TARGET ncursesw)
endif()

if(NCURSESW_FOUND)
    target_link_libraries(net-stats PRIVATE PkgConfig::NCURSESW)
else()
    set(CURSES_NEED_NCURSES TRUE)
    set(CURSES_NEED_WIDE TRUE)
    find_package(Curses REQUIRED)

    target_include_directories(net-stats PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(net-stats PRIVATE ${CURSES_LIBRARIES})
endif()

install(TARGETS net-stats RUNTIME DESTINATION bin)
