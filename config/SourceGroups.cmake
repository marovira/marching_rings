#=============================================================================#
# Source group config.
#=============================================================================#
source_group("include" FILES ${ATHENA_INCLUDE_TOP_GROUP})
source_group("include\\athena" FILES)
source_group("include\\athena\\core" FILES ${ATHENA_INCLUDE_CORE_GROUP})
source_group("include\\athena\\fields" FILES ${ATHENA_INCLUDE_FIELDS_GROUP})
source_group("include\\athena\\tree" FILES ${ATHENA_INCLUDE_TREE_GROUP})
source_group("include\\athena\\blob" FILES ${ATHENA_INCLUDE_BLOB_GROUP})

source_group("source" FILES ${ATHENA_SOURCE_TOP_GROUP})
source_group("source\\athena" FILES)
source_group("source\\athena\\core" FILES ${ATHENA_SOURCE_CORE_GROUP})
source_group("source\\athena\\tree" FILES ${ATHENA_SOURCE_TREE_GROUP})
source_group("source\\athena\\blob" FILES ${ATHENA_SOURCE_BLOB_GROUP})
