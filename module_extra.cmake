set(sunset_tld ${NAP_ROOT}/modules/napsunset/thirdparty/sunset)
if(WIN32)
    set(SUNSET_DIR ${sunset_tld}/msvc/x86_64)
elseif(UNIX)
    set(SUNSET_DIR ${sunset_tld}/linux/${ARCH})
endif()

# set (SUNSET_DEBUGTEST Z:/NapLabs/nap/modules/napsunset/thirdparty/sunset/msvc/x86_64/include/sunset)

target_include_directories(${PROJECT_NAME} PUBLIC ${SUNSET_DIR}/include/sunset)
# target_include_directories(${PROJECT_NAME} PUBLIC ${SUNSET_DEBUGTEST})



# if(WIN32)
#     set(CUSTOM_SUNSET_LIBS_DIR ${SUNSET_DIR}/x64/vc16/bin)
#     set(CUSTOM_SUNSET_DEBUG_DLL ${SUNSET_DIR}/x64/vc16/bin/sunset.dll)
#     set(CUSTOM_SUNSET_RELEASE_DLL ${SUNSET_DIR}/x64/vc16/bin/sunset.dll)
# endif()


# add_library(customSunset SHARED IMPORTED)

# set_target_properties(customSunset PROPERTIES
#                       IMPORTED_CONFIGURATIONS "Debug;Release;"
#                       IMPORTED_LOCATION_RELEASE ${CUSTOM_SUNSET_RELEASE_DLL}
#                       IMPORTED_LOCATION_DEBUG ${CUSTOM_SUNSET_RELEASE_DLL}
#                       )

# if(WIN32)
#     # Copy over DLLs post-build on Windows
#     file(GLOB SunsetDLL ${SUNSET_DIR}/x64/vc16/bin/*.dll)
#     set(DLLCOPY_PATH_SUFFIX "")
#     add_custom_command(
#         TARGET ${PROJECT_NAME}
#         POST_BUILD
#         COMMAND ${CMAKE_COMMAND} 
#         -E copy ${CUSTOM_SUNSET_DEBUG_DLL} 
#         $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
#     )

#     add_custom_command(
#         TARGET ${PROJECT_NAME}
#         POST_BUILD
#         COMMAND ${CMAKE_COMMAND} 
#         -E copy ${CUSTOM_SUNSET_RELEASE_DLL} 
#         $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
#     )
# endif()