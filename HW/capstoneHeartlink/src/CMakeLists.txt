# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(
  SRCS
    "main.c"
    "../hal/src/hal_max30102_sensor.c"
    "../hal/src/hal_reg.c"
    "../hal/src/hal_i2c.c"
    "../max30102_driver/src/max30102_driver.c"
  INCLUDE_DIRS
    "."
    "../include"
    "../hal/include"
    "../max30102_driver/include"
  REQUIRES
    driver
)