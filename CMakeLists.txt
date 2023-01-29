cmake_minimum_required(VERSION 3.10)

project(CitraPad)

find_package(SDL2)
if(NOT SDL2_FOUND)
  include(ExternalProject)
  ExternalProject_Add(SDL
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL"
    GIT_TAG "release-2.26.2"
    CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_CURRENT_BINARY_DIR}/deps
  )
  set(SDL2_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/deps/include/SDL2)

  add_library(SDL2::SDL2main SHARED IMPORTED)
  set_target_properties(SDL2::SDL2main PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/deps/bin/SDL2.dll
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/deps/bin/SDL2d.dll
    IMPORTED_IMPLIB ${CMAKE_CURRENT_BINARY_DIR}/deps/lib/SDL2main.lib
    IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/deps/lib/SDL2maind.lib
  )

  add_library(SDL2::SDL2 SHARED IMPORTED)
  set_target_properties(SDL2::SDL2 PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/deps/bin/SDL2.dll
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/deps/bin/SDL2d.dll
    IMPORTED_IMPLIB ${CMAKE_CURRENT_BINARY_DIR}/deps/lib/SDL2.lib
    IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/deps/lib/SDL2d.lib
  )
endif(NOT SDL2_FOUND)

if(WIN32)
  add_executable(citrapad
    src/citrapad.h
    src/citrapad.c
    src/citrapad_win.c
    src/joystick.h
    src/joystick.c
    src/main.c)
  target_include_directories(citrapad PUBLIC ${SDL2_INCLUDE_DIRS})
  target_link_libraries(citrapad PUBLIC SDL2::SDL2main SDL2::SDL2 wsock32 ws2_32)
else()
  add_executable(citrapad
    src/citrapad.h
    src/citrapad.c
    src/citrapad_posix.c
    src/joystick.h
    src/joystick.c
    src/main.c)
  target_include_directories(citrapad PUBLIC ${SDL2_INCLUDE_DIRS})
  target_link_libraries(citrapad PUBLIC SDL2::SDL2main SDL2::SDL2)
endif(WIN32)
