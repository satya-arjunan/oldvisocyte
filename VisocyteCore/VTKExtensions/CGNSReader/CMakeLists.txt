set(classes
  vtkCGNSFileSeriesReader
  vtkCGNSReader
  vtkCGNSSubsetInclusionLattice)

set(sources
  cgio_helpers.cxx
  vtkCGNSReaderInternal.cxx
  vtkFileSeriesHelper.cxx)

set(private_headers
  cgio_helpers.h
  vtkCGNSCache.h
  vtkCGNSReaderInternal.h
  vtkFileSeriesHelper.h)

vtk_module_add_module(Visocyte::VTKExtensionsCGNSReader
  CLASSES ${classes}
  SOURCES ${sources}
  PRIVATE_HEADERS ${private_headers})

visocyte_add_server_manager_xmls(
  XMLS  resources/CGNSReader.xml)
