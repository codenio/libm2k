if (@ENABLE_LOG@)
    include(CMakeFindDependencyMacro)
    find_dependency(glog)
endif()
include("${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@Targets.cmake")
