add_compile_options(-O0)

file(GLOB src "*.cpp")
list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
message("${src}")
add_library(cuda_sync_analyzer SHARED ${src})

configure_file (
    "SyncDriverVerisons.txt"
    "${CMAKE_INSTALL_PREFIX}/lib/SyncDriverVerisons.txt"
)
configure_file (
    "${PROJECT_SOURCE_DIR}/src/Constants.h.in"
    "${PROJECT_SOURCE_DIR}/src/Constants.h"
)

target_link_libraries(cuda_sync_analyzer PUBLIC "-lcrypto\
    -L${BOOST_LIBRARYDIR} -lboost_filesystem -lboost_system\
    ${DYNINST_LIBRARIES}")
    #    -L${CUDA_TOOLKIT_ROOT_DIR}/lib64\
target_include_directories(cuda_sync_analyzer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS cuda_sync_analyzer
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        RUNTIME DESTINATION bin)

add_subdirectory(FindSyncHelper)
add_subdirectory(InsertTimingInstr)

add_executable(mutateLibcuda main.cpp)
link_directories(${CMAKE_BINARY_DIR}/src)
target_link_libraries(mutateLibcuda LINK_PUBLIC "cuda_sync_analyzer")
install(TARGETS mutateLibcuda
        RUNTIME DESTINATION bin)
