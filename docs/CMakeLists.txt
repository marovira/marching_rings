add_custom_target(athena_docs ALL ${DOXYGEN_EXECUTABLE}
    WORKING_DIRECTORY ${ATHENA_BINARY_DIR}/docs
    COMMENT "Generating HTML documentation" VERBATIM)
set_target_properties(athena_docs PROPERTIES FOLDER "athena")
