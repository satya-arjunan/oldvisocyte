cmake_minimum_required(VERSION 3.8)
project(Demo0)

find_package(Visocyte REQUIRED)

if (NOT VISOCYTE_BUILD_QT_GUI)
  message(STATUS
    "${CMAKE_PROJECT_NAME} example requires VISOCYTE_BUILD_QT_GUI to be "
    "enabled. Please rebuild Visocyte (or point to a different build of "
    "Visocyte) with VISOCYTE_BUILD_QT_GUI set to TRUE")
  return ()
endif()

include(GNUInstallDirs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(DemoApp0 DemoApp0.cxx)
target_link_libraries(DemoApp0
  PRIVATE
    Visocyte::pqCore
    Qt5::Widgets)
