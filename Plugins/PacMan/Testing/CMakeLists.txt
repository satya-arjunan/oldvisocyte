set(module_tests
  LoadPlugins.xml)

if (VISOCYTE_BUILD_QT_GUI AND BUILD_SHARED_LIBS)
  visocyte_add_client_tests("pv"
    BASELINE_DIR ${VISOCYTE_TEST_BASELINE_DIR}
    TEST_DATA_TARGET VisocyteData
    TEST_SCRIPTS ${MODULE_TESTS}
  )
endif ()
