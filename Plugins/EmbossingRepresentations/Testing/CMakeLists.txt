include(VisocyteTesting)

if (VISOCYTE_BUILD_QT_GUI AND BUILD_SHARED_LIBS)
  visocyte_add_client_tests(
    LOAD_PLUGIN "EmbossingRepresentations"
    BASELINE_DIR ${VISOCYTE_TEST_BASELINE_DIR}
    TEST_DATA_TARGET VisocyteData
    TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/ExtrusionRepresentationPointData.xml)
  visocyte_add_client_tests(
    LOAD_PLUGIN "EmbossingRepresentations"
    BASELINE_DIR ${VISOCYTE_TEST_BASELINE_DIR}
    TEST_DATA_TARGET VisocyteData
    TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/ExtrusionRepresentationCellData.xml)
  visocyte_add_client_tests(
    LOAD_PLUGIN "EmbossingRepresentations"
    BASELINE_DIR ${VISOCYTE_TEST_BASELINE_DIR}
    TEST_DATA_TARGET VisocyteData
    TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/BumpMapRepresentationPointData.xml)
endif()
