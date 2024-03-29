Environment Variables       {#EnvironmentVariables}
=====================

This page documents environment variables that affect Visocyte behavior at
runtime.

Variable | Description
---------|---------------------------------------------------------
VISOCYTE_DATA_ROOT  | Change the location of the data root for testing.
PV_DEBUG_PANELS | When set, debugging text will be printed out explaining the reason for creation of various widgets on the properties panel (pqPropertiesPanel).
PV_DEBUG_SKIP_OPENGL_VERSION_CHECK | Skip test to validate OpenGL support at launch.
PV_DEBUG_TEST | Prints debugging information about the testing framework during playback to cout.
PV_ICET_WINDOW_BORDERS | Force render windows to be 400x400 instead of fullscreen.
PV_DEBUG_REMOTE_RENDERING | Forces server-side render windows to swap buffers in order to see what is being rendered on the server ranks.
PV_MACRO_PATH | Additional directories defined by the user to store macros.
PV_ALLOW_BATCH_INTERACTION | Allow interactions in batch mode.
PV_PLUGIN_CONFIG_FILE | XML Plugin Configuration Files to specify which plugin to load on startup.
PV_PLUGIN_PATH | Directories containing plugins to be loaded on startup.
QT_MAC_NO_NATIVE_MENUBAR | Qt flag to force the Qt menu bar rather than the native mac menu bar.
PV_PLUGIN_DEBUG | (obsolete) Use **VISOCYTE_LOG_PLUGIN_VERBOSITY** instead.
PV_SETTINGS_DEBUG | (obsolete) Use **VISOCYTE_LOG_APPLICATION_VERBOSITY** instead.
PV_DEBUG_APPLY_BUTTON | (obsolete) Use **VISOCYTE_LOG_APPLICATION_VERBOSITY** instead.

Visocyte supports generating logs that includes debugging and tracking
information. The log messages are categorized and it is possible to temporarily
elevate the log level for any category using the following environment
variables. The value for each of the variables can be a number in the range
[-2, 9] or the strings `INFO`, `ERROR`, `WARNING`, `TRACE`, or `MAX` (See
`vtkLogger` for additional information about log levels).

Variable | Description
---------|-----------------------------------------
VISOCYTE_LOG_PLUGIN_VERBOSITY | Log messages related to Visocyte plugins (see vtkPVLogger::GetPluginVerbosity())
VISOCYTE_LOG_PIPELINE_VERBOSITY  | Log messages related to Pipeline execution (see vtkPVLogger::GetPipelineVerbosity())
VISOCYTE_LOG_DATA_MOVEMENT_VERBOSITY | Log messages related to data movement for rendering and other tasks (see vtkPVLogger::GetDataMovementVerbosity())
VISOCYTE_LOG_APPLICATION_VERBOSITY | Log messages related to the application (see vtkPVLogger::GetApplicationVerbosity())
