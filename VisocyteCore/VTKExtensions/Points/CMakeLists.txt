set(classes
  vtkBoundedPlaneSource
  vtkBoundedVolumeSource)

vtk_module_add_module(Visocyte::VTKExtensionsPoints
  CLASSES ${classes})

visocyte_add_server_manager_xmls(
  XMLS  points.xml)
