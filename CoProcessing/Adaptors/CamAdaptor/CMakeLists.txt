cmake_minimum_required(VERSION 3.3)
project(CamAdaptor CXX)

find_package(Visocyte REQUIRED COMPONENTS vtkPVPythonCatalyst)

INCLUDE(${VISOCYTE_USE_FILE})
if(NOT VISOCYTE_USE_MPI)
  message(SEND_ERROR "Visocyte must be built with MPI enabled")
endif()

configure_file(cesm1_2_2.patch.in cesm1_2_2.patch @ONLY)

ADD_LIBRARY(CamAdaptor
  Grid.cxx
  fv_create_data.cxx
  se_create_data.cxx)
TARGET_LINK_LIBRARIES(CamAdaptor vtkPVPythonCatalyst)

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Woverloaded-virtual -Wno-deprecated")
endif()
