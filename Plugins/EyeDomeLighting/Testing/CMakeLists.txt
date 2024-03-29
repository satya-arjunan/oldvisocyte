set(EYE_DOME_LIGHTING_TESTS
  EyeDomeLighting.xml
)

if (VISOCYTE_BUILD_QT_GUI AND BUILD_SHARED_LIBS)
  visocyte_add_client_tests(
    LOAD_PLUGIN "EyeDomeLighting"
    BASELINE_DIR ${VISOCYTE_TEST_BASELINE_DIR}
    TEST_DATA_TARGET VisocyteData
    TEST_SCRIPTS ${EYE_DOME_LIGHTING_TESTS}
  )
endif ()


if (TARGET Visocyte::pvpython)
  # this test is not specific to the EyeDomeLighting plugin, it is meant
  # to test that we can reload a plugin multiple times without a crash.
  # for complex Python scripts we may not know that a plugin has already
  # been loaded.
  add_test(NAME ReloadPlugin
    COMMAND $<TARGET_FILE:Visocyte::pvpython>
            "${CMAKE_CURRENT_SOURCE_DIR}/ReloadPlugin.py")
  set_tests_properties(ReloadPlugin PROPERTIES LABELS EyeDomeLighting)
endif ()
