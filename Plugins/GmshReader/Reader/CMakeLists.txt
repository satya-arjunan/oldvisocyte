set(classes
  vtkGmshMetaReader
  vtkGmshReader)

set(private_headers
  vtkGmshIncludes.h)

vtk_module_find_package(
  PACKAGE Gmsh
  VERSION 4.0.0)

vtk_module_add_module(GmshReader::vtkGmshReader
  CLASSES ${classes}
  PRIVATE_HEADERS ${private_headers})
vtk_module_link(GmshReader::vtkGmshReader
  PRIVATE
    Gmsh::Gmsh)

visocyte_add_server_manager_xmls(
  XMLS  GmshReaderServerManager.xml)
