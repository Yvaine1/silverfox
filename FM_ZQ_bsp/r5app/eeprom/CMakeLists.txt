project(eeprom)
cmake_minimum_required(VERSION 3.16)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/header
  $ENV{DRIVER_HEADER_PATH}
  )

set(EEPROM_API_SRCS
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../i2c/common/src/i2c_common.c
    src/eeprom_api.c
    src/eeprom_interface.c
    src/crc32.c
  )
add_compile_definitions(SONAME="libdgeeprom.so")
link_directories($ENV{DRIVER_SO_PATH}) #添加动态连接库的路径
add_definitions(-DBOARD_PRJ=$ENV{OPT_BOARD_PRJ})
add_library(dgeeprom SHARED ${EEPROM_API_SRCS})
target_link_libraries(dgeeprom -ldl -ldgdg_utils -ldgboard_driver_common)

add_definitions(-Wno-inline -Werror -Wall)

set(EEPROM_BIN_SRCS
    src/eeprom_main.c
  )
add_executable(eeprom ${EEPROM_BIN_SRCS})
target_link_libraries(eeprom dgeeprom)
set_property(TARGET dgeeprom PROPERTY C_STANDARD 99)
