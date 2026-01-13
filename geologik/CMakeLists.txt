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
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")

  target_compile_options(${PROJECT_NAME} PRIVATE /MP)

  target_link_options(${PROJECT_NAME} 
    PRIVATE /NODEFAULTLIB:MSVCRT
    PRIVATE /NODEFAULTLIB:LIBCMT
  )
endif()

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /WX /W4)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra)
endif()

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    _CRT_SECURE_NO_WARNINGS
    SSE2
    GLEW_STATIC
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

if (WIN32)
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE
      $<$<CONFIG:Release>:WIN32>
      $<$<CONFIG:Release>:_WINDOWS>
  )
endif()

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/SDL2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/glew/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/stb/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
)

if (WIN32)
  target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/SDL2/lib)

  target_link_libraries(${PROJECT_NAME} PRIVATE 
    SDL2.lib SDL2main.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/glew/lib/libglew32.lib
    opengl32.lib
    glu32.lib
  )

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${CMAKE_BINARY_DIR}"
  )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME_DEBUG   "${PROJECT_NAME}D" # may be bullshit
  OUTPUT_NAME_RELEASE "${PROJECT_NAME}" # may be bullshit
)

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE
    /EHsc
    /GR-
    /fp:fast
    $<$<CONFIG:Debug>:/Od>
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Release>:/Ob2>
    $<$<CONFIG:Release>:/GR->
  )

  target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:/DEBUG:FASTLINK>
    $<$<CONFIG:Release>:/INCREMENTAL:NO>
  )
endif()

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        kernel32
        user32
        gdi32
        winspool
        comdlg32
        advapi32
        shell32
        ole32
        oleaut32
        uuid
        odbc32
        odbccp32
        winmm
        setupapi
        version
        Imm32
        Ws2_32
        Wldap32
        Crypt32
    )
endif()
