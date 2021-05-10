cmake_minimum_required(VERSION 3.10)

project(CitraPad)
add_subdirectory(SDL)

add_executable(citra_pad main.c)

if(WIN32)
  target_link_libraries(citra_pad PUBLIC SDL2::SDL2main SDL2::SDL2 wsock32 ws2_32)
else()
  target_link_libraries(citra_pad PUBLIC SDL2::SDL2main SDL2::SDL2)
endif(WIN32)