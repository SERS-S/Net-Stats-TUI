cmake_minimum_required(VERSION 3.10)
project(net_stats_tui LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

file(GLOB SRC_DATA "${CMAKE_CURRENT_SOURCE_DIR}/src/data/*.c")
file(GLOB SRC_DRAW "${CMAKE_CURRENT_SOURCE_DIR}/src/draw/*.c")
file(GLOB SRC_REACTOR "${CMAKE_CURRENT_SOURCE_DIR}/src/reactor/*.c")

add_executable(net-stats
    src/main.c
    ${SRC_REACTOR}
    ${SRC_DATA}
    ${SRC_DRAW}  
)

target_include_directories(net-stats PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(NCURSESW QUIET IMPORTED_TARGET ncursesw)
    pkg_check_modules(LIBNL3 QUIET IMPORTED_TARGET libnl-3.0)
    pkg_check_modules(LIBNLGENL3 QUIET IMPORTED_TARGET libnl-genl-3.0)
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

if(LIBNL3_FOUND AND LIBNLGENL3_FOUND)
    target_link_libraries(net-stats PRIVATE PkgConfig::LIBNL3 PkgConfig::LIBNLGENL3)
    target_compile_definitions(net-stats PRIVATE NET_STATS_HAS_LIBNL)
endif()

install(TARGETS net-stats RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
