set(module_tests
  SLACTools.xml
)

if (VISOCYTE_BUILD_QT_GUI AND BUILD_SHARED_LIBS)
  ExternalData_Expand_Arguments("VisocyteData" _
    "DATA{${CMAKE_CURRENT_SOURCE_DIR}/Data/Baseline/,REGEX:.*}"
    "DATA{${CMAKE_CURRENT_SOURCE_DIR}/Data/SLAC/pic-example/,REGEX:.*}"
    "DATA{${CMAKE_CURRENT_SOURCE_DIR}/Data/SLAC/pillbox/,REGEX:.*}")

  set(SLACTools_THRESHOLD 70)

  visocyte_add_client_tests(
    LOAD_PLUGIN "SLACTools"
    TEST_DATA_TARGET VisocyteData
    TEST_SCRIPTS ${MODULE_TESTS}
  )
endif ()
