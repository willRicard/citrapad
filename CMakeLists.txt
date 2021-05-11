cmake_minimum_required(VERSION 3.10)

project(CitraPad)
add_subdirectory(SDL)

if(WIN32)
  add_executable(citrapad
    src/citrapad.h
    src/citrapad.c
    src/citrapad_win.c
    src/joystick.h
    src/joystick.c
    src/main.c)
  target_link_libraries(citrapad PUBLIC SDL2::SDL2main SDL2::SDL2 wsock32 ws2_32)
else()
  add_executable(citrapad
    src/citrapad.h
    src/citrapad.c
    src/citrapad_posix.c
    src/joystick.h
    src/joystick.c
    src/main.c)
  target_link_libraries(citrapad PUBLIC SDL2::SDL2main SDL2::SDL2)
endif(WIN32)

