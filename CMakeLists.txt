cmake_minimum_required(VERSION 3.0)

project(Allotemplate)

# name of application. replace 'app' with desired app name
set(APP_NAME app)

# path to main source file
add_executable(${APP_NAME} src/main.cpp)

# add allolib as a subdirectory to the project
add_subdirectory(allolib)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/al_ext)
  message("Buiding extensions in al_ext")
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/al_ext)
  get_target_property(AL_EXT_LIBRARIES al_ext AL_EXT_LIBRARIES)
  target_link_libraries(${APP_NAME} PRIVATE ${AL_EXT_LIBRARIES})
endif()

# link allolib to project
target_link_libraries(${APP_NAME} PRIVATE al)

target_include_directories(${APP_NAME} PRIVATE /usr/local/include/libfreenect2)

target_link_libraries(${APP_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/libfreenect2.dylib)

# binaries are put into the ./bin directory by default
set_target_properties(${APP_NAME} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin
)
