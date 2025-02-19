cmake_minimum_required(VERSION 3.16)
project(demo)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# Enable Blend2D static build.
set(BLEND2D_STATIC TRUE)
set(BLEND2D_DIR "${CMAKE_CURRENT_LIST_DIR}/vendored/blend2d"
    CACHE PATH "Location of 'blend2d'")
include("${BLEND2D_DIR}/CMakeLists.txt")

set(SDL_STATIC TRUE)

# Create your game executable target as usual
add_executable(demo WIN32 src/main.c src/blend.c src/sdl.c src/flecs.c)
if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
  set_target_properties(demo PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_CURRENT_LIST_DIR}/assets@assets -s ALLOW_MEMORY_GROWTH=1 -s STACK_SIZE=1mb")
endif()

target_include_directories(demo PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include")

# Link to the actual SDL3 library.
target_link_libraries(demo PRIVATE SDL3::SDL3 blend2d::blend2d)