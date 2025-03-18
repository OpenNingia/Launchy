add_subdirectory(common)
add_subdirectory(src)

if (BUILD_PLUGINS)
    add_subdirectory(plugins)
endif(BUILD_PLUGINS)