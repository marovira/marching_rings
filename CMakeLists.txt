#=============================================================================#
# Project setup.
#=============================================================================#
project(athena)

cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0054 NEW)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# We do not support Apple, so disable it from the get-go.
if (APPLE)
    message(FATAL_ERROR "Athena only supports Windows and Linux.")
endif()

# Any compile-time options go here.
option(ATHENA_BUILD_DOCS "Build marching rings documentation" ON)
option(ATHENA_PARALLEL "Enable parallelization using TBB" ON)

# Set the version data.
set(ATHENA_VERSION_MAJOR "0")
set(ATHENA_VERSION_MINOR "0")
set(ATHENA_VERSION_PATCH "0")
set(ATHENA_VERSION_EXTRA "")
set(ATHENA_VERSION "${ATHENA_VERSION_MAJOR}.${ATHENA_VERSION_MINOR}")
set(ATHENA_VERSION_FULL 
    "${ATHENA_VERSION}.${ATHENA_VERSION_PATCH}${ATHENA_VERSION_EXTRA}")

#=============================================================================#
# Directory Variables.
#=============================================================================#
set(ATHENA_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
set(ATHENA_GLM_ROOT "${ATHENA_SOURCE_DIR}/external/glm")
set(ATHENA_INCLUDE_ROOT "${ATHENA_SOURCE_DIR}/include")
set(ATHENA_SOURCE_ROOT "${ATHENA_SOURCE_DIR}/source")
set(ATHENA_DOCS_ROOT "${ATHENA_SOURCE_DIR}/docs")
set(ATHENA_CONFIG_ROOT "${ATHENA_SOURCE_DIR}/config")

#=============================================================================#
# Compilation settings.
#=============================================================================#
include("${ATHENA_CONFIG_ROOT}/Compiler.cmake")
include("${ATHENA_CONFIG_ROOT}/Externals.cmake")

#=============================================================================#
# Generate files.
#=============================================================================#
if (ATHENA_BUILD_DOCS)
    configure_file("${ATHENA_DOCS_ROOT}/Doxyfile.in"
        "${ATHENA_BINARY_DIR}/docs/Doxyfile" @ONLY)
endif()

#=============================================================================#
# Subdirectories.
#=============================================================================#
add_subdirectory("${ATHENA_INCLUDE_ROOT}/athena")
add_subdirectory("${ATHENA_SOURCE_ROOT}")

if (DOXYGEN_FOUND AND ATHENA_BUILD_DOCS)
    add_subdirectory("${ATHENA_DOCS_ROOT}")
endif()

#=============================================================================#
# Include directories.
#=============================================================================#
include_directories(
    ${ATHENA_INCLUDE_ROOT}
    ${ATHENA_GLM_ROOT}
    )

#=============================================================================#
# Executable.
#=============================================================================#
include("${ATHENA_CONFIG_ROOT}/SourceGroups.cmake")
add_executable(athena ${ATHENA_SOURCE_LIST} ${ATHENA_INCLUDE_LIST})

# Only link with TBB if we are using a parallel build.
if (ATHENA_PARALLEL)
    target_link_libraries(athena tbb)
endif()

set_target_properties(athena PROPERTIES FOLDER "athena")
