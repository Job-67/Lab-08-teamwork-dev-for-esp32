set(COMPONENT_SRCS
    "LED/led.c"
)

set(COMPONENT_ADD_INCLUDEDIRS
    "LED"
)

# Register all components in this directory
idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}

)