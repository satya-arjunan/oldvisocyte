# This plugin contains tools developed for the Stanford Linear Accelerator
# Center to make it easier to visualize finite element simulation results.

set(ui_files)
set(qrc_files)
set(sources)
set(interfaces)
if (VISOCYTE_BUILD_QT_GUI)
  list(APPEND ui_files
    pqSLACActionHolder.ui
    pqSLACDataLoadManager.ui)

  list(APPEND qrc_files
    SLACTools.qrc)

  list(APPEND sources
    pqSLACActionGroup.cxx
    pqSLACActionGroup.h
    pqSLACDataLoadManager.cxx
    pqSLACDataLoadManager.h
    pqSLACManager.cxx
    pqSLACManager.h)

  visocyte_plugin_add_action_group(
    CLASS_NAME pqSLACActionGroup
    GROUP_NAME "ToolBar/SLAC"
    INTERFACES action_interfaces
    SOURCES action_sources)
  list(APPEND interfaces
    ${action_interfaces})
  list(APPEND sources
    ${action_sources})
endif ()

visocyte_add_plugin(SLACTools
  VERSION "1.1"
  UI_FILES ${ui_files}
  UI_INTERFACES ${interfaces}
  UI_RESOURCES ${qrc_files}
  SOURCES ${sources}
  MODULES SLACTools::vtkSLACFilters
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtk.module")

if (VISOCYTE_BUILD_QT_GUI)
  target_link_libraries(SLACTools
    PRIVATE
      Visocyte::ClientServerCoreCore
      Visocyte::ServerManagerCore
      Visocyte::ServerManagerRendering
      VTK::CommonCore
      VTK::CommonDataModel
      VTK::CommonExecutionModel)
endif ()

if (BUILD_TESTING)
  add_subdirectory(Testing)
endif ()
