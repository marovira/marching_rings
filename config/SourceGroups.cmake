#=============================================================================#
# Source group config.
#=============================================================================#
source_group("include" FILES ${ATHENA_INCLUDE_TOP_GROUP})
source_group("include\\athena" FILES)
source_group("include\\athena\\fields" FILES ${ATHENA_INCLUDE_FIELDS_GROUP})
source_group("include\\athena\\operators" FILES
    ${ATHENA_INCLUDE_OPERATORS_GROUP})
source_group("include\\athena\\tree" FILES ${ATHENA_INCLUDE_TREE_GROUP})
source_group("include\\athena\\polygonizer" FILES 
    ${ATHENA_INCLUDE_POLYGONIZER_GROUP})
source_group("include\\athena\\visualizer" FILES
    ${ATHENA_INCLUDE_VISUALIZER_GROUP})
source_group("include\\athena\\models" FILES ${ATHENA_INCLUDE_MODELS_GROUP})

source_group("source" FILES ${ATHENA_SOURCE_TOP_GROUP})
source_group("source\\athena" FILES)
source_group("source\\athena\\tree" FILES ${ATHENA_SOURCE_TREE_GROUP})
source_group("source\\athena\\polygonizer" FILES 
    ${ATHENA_SOURCE_POLYGONIZER_GROUP})
source_group("source\\athena\\visualizer" FILES
    ${ATHENA_SOURCE_VISUALIZER_GROUP})
source_group("source\\athena\\models" FILES ${ATHENA_SOURCE_MODELS_GROUP})

source_group("shader" FILES ${ATHENA_SHADER_TOP_GROUP})
source_group("shader\\athena" FILES)
source_group("shader\\athena\\global" FILES ${ATHENA_SHADER_GLOBAL_GROUP})
source_group("shader\\athena\\visualizer" FILES
    ${ATHENA_SHADER_VISUALIZER_GROUP})
