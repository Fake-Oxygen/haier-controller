cmake_minimum_required(VERSION 3.20.0)
set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/secrets.conf")
  list(APPEND EXTRA_CONF_FILE "${CMAKE_CURRENT_LIST_DIR}/secrets.conf")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(haier-controller)

target_include_directories(app PRIVATE inc)
target_sources(app PRIVATE 
  src/main.c 
  src/wifi.c
  src/shell.c
  src/relay.c
  src/rom.c
)

add_subdirectory(drivers)
