vtk_module_test_data(
  Data/dualSphereAnimation/,REGEX:.*
  Data/dualSphereAnimation.pvd)

add_subdirectory(Cxx)

if (VISOCYTE_ENABLE_PYTHON)
  vtk_module_test_data(
    Data/can.ex2)

  add_subdirectory(Python)
endif ()
