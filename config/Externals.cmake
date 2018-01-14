#=============================================================================#
# External dependencies.
#=============================================================================#
if (MR_BUILD_DOCS)
    # Skip the graph stuff from Doxygen.
    set(DOXYGE_SKIP_DOT TRUE)
    find_package(Doxygen)
endif()

list(APPEND CMAKE_MODULE_PATH ${ATHENA_CONFIG_ROOT})
if (ATHENA_PARALLEL)
    find_package(TBB REQUIRED)
endif()
