set(module_tests
  VTKmClip.xml
  VTKmContour.xml
  VTKmExternalFaces.xml
  VTKmGradient.xml
  VTKmGradientUG.xml
  VTKmThreshold.xml
)

if (VISOCYTE_BUILD_QT_GUI AND BUILD_SHARED_LIBS)
  visocyte_add_client_tests(
    LOAD_PLUGIN "VTKmFilters"
    BASELINE_DIR ${VISOCYTE_TEST_BASELINE_DIR}
    TEST_DATA_TARGET VisocyteData
    TEST_SCRIPTS ${module_tests}
  )
endif ()
