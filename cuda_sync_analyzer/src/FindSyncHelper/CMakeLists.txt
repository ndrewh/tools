add_compile_options(-O3 -g)
#add_compile_options(-mno-vsx)
add_library(FindSyncHelper SHARED "FindSyncHelper.cpp")
#target_link_libraries(FindSyncHelper ${Boost_LIBRARIES})
#target_link_libraries(FindSyncHelper ${DYNINST_LIBRARIES})
#target_link_libraries(FindSyncHelper "-ldyninstAPI_RT")
#target_link_libraries(FindSyncHelper "-lpthread")
#target_link_libraries(FindSyncHelper DriverAPIWrapper)
#target_link_libraries(FindSyncHelper PluginCommon)
#INCLUDE_DIRECTORIES(FindSyncHelper ${CUDA_TOOLKIT_ROOT_DIR}/include)
#INCLUDE_DIRECTORIES(FindSyncHelper ${CUDA_TOOLKIT_ROOT_DIR}/extras/CUPTI/include)
#INCLUDE_DIRECTORIES(FindSyncHelper ${Boost_INCLUDE_DIRS})
#INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/common")
#INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/driverapi")
#INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/deduplicate")
#INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/driverapi/launcher")
#INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/driverapi/plugins/common")
#target_link_libraries(FindSyncHelper "-L${GOTTCHA_LIBRARIES} -lgotcha" )
#INCLUDE_DIRECTORIES(FindSyncHelper ${GOTTCHA_INCLUDE})
#target_link_libraries(FindSyncHelper "-L${CUDA_TOOLKIT_ROOT_DIR}/lib64 -lcudart -L${CUDA_TOOLKIT_ROOT_DIR}/extras/CUPTI/lib64 -lcupti -lboost_timer")
install(TARGETS FindSyncHelper
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
