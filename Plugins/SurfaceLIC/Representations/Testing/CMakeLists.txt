# TODO: Needs updated. These have effectively been disabled previously.
if (FALSE)
if (VISOCYTE_ENABLE_PYTHON)
  vtk_module_test_data(
    Data/shuttle-surf.vtkm
    Data/office.binary.vtk
    Data/shuttle-surf/,REGEX:.*)

  add_subdirectory(Python)
endif ()
endif ()
