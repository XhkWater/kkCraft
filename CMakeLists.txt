cmake_minimum_required(VERSION 3.20)

project(kkCraft)

set(CMAKE_CXX_STANDARD 20)

# libs
include_directories(D:/VulkanSDK/1.4.341.1/Include)
link_directories(D:/VulkanSDK/1.4.341.1/Lib)

include_directories(D:/sdl/Include)
link_directories(D:/sdl/Lib)

file(GLOB SOURCES "src/*.cpp")

add_executable(kkCraft 
${SOURCES}
)

# Keep executable in build/ for multi-config generators (e.g. Visual Studio).
set_target_properties(kkCraft PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}"
)

find_program(GLSLC_EXECUTABLE NAMES glslc HINTS "$ENV{VULKAN_SDK}/Bin")
if(NOT GLSLC_EXECUTABLE)
  message(FATAL_ERROR "glslc not found. Please install Vulkan SDK and ensure glslc is available.")
endif()

set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_BINARY_DIR "${CMAKE_BINARY_DIR}/assets")
set(SHADER_SOURCE_DIR "${ASSETS_SOURCE_DIR}/shaders")
set(SHADER_WORK_DIR "${ASSETS_BINARY_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY "${SHADER_BINARY_DIR}")

# Copy the full assets folder into the build output.
add_custom_target(copy_assets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${ASSETS_SOURCE_DIR}"
          "${ASSETS_BINARY_DIR}"
  COMMENT "Copying assets to build directory"
)

set(SHADER_SOURCES
  "${SHADER_SOURCE_DIR}/triangle.vert"
  "${SHADER_SOURCE_DIR}/triangle.frag"
)

set(SHADER_BINARIES)
foreach(SHADER ${SHADER_SOURCES})
  get_filename_component(SHADER_FILE_NAME ${SHADER} NAME)
  set(SHADER_OUTPUT "${SHADER_BINARY_DIR}/${SHADER_FILE_NAME}.spv")
  add_custom_command(
    OUTPUT "${SHADER_OUTPUT}"
    COMMAND "${GLSLC_EXECUTABLE}" "${SHADER_WORK_DIR}/${SHADER_FILE_NAME}" -o "${SHADER_OUTPUT}"
    DEPENDS copy_assets "${SHADER}"
    COMMENT "Compiling shader ${SHADER_FILE_NAME}"
    VERBATIM
  )
  list(APPEND SHADER_BINARIES "${SHADER_OUTPUT}")
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SHADER_BINARIES})
add_dependencies(kkCraft compile_shaders)

target_compile_definitions(kkCraft PRIVATE SHADER_DIR="${SHADER_BINARY_DIR}")

target_link_libraries(kkCraft PRIVATE
vulkan-1.lib
SDL2.lib
)

if (WIN32)
  target_link_libraries(kkCraft PRIVATE dwmapi.lib)
endif()