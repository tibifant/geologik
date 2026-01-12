file(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.rc"
)

file(GLOB_RECURSE PROJECT_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.inl"
)

file(GLOB PROJECT_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*.vert"
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_SHADERS}
)

if (WIN32 AND MSVC)
  # Use the DLL runtime (/MD, /MDd)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")

  # Multi-processor compilation
  target_compile_options(${PROJECT_NAME} PRIVATE /MP)

  target_link_options(${PROJECT_NAME} PRIVATE /NODEFAULTLIB:MSVCRT)
  target_link_options(${PROJECT_NAME} PRIVATE /NODEFAULTLIB:LIBCMT)
endif()

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /WX) 
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Werror)
endif()

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    _CRT_SECURE_NO_WARNINGS
    SSE2
    GLEW_STATIC
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/SDL2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/glew/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/stb/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/builds/bin"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/builds/bin"
)

if (WIN32)
  target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/SDL2/lib)

  target_link_libraries(${PROJECT_NAME} PRIVATE 
    SDL2.lib SDL2main.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/glew/lib/libglew32.lib
    opengl32.lib
    glu32.lib
  )
endif()
