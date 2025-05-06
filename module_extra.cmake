# thirdparty include & source
set(SUNSET_DIR ${NAP_ROOT}/modules/napsunset/thirdparty/sunset)

# sunset library source, group and include
# note that the nap module doesn't expose the interface
set(SUNSET_CPP ${SUNSET_DIR}/include/sunset.cpp)
source_group("Sunset" FILES ${SUNSET_CPP})
target_sources(${PROJECT_NAME} PRIVATE ${SUNSET_CPP})
target_include_directories(${PROJECT_NAME} PRIVATE ${SUNSET_DIR}/include)

# install sunset license
install(FILES ${SUNSET_DIR}/LICENSE DESTINATION licenses/sunset)