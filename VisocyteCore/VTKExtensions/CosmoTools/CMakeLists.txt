# Resolve the GenericIO external dependency
find_package(GenericIO REQUIRED)

# List of module sources
set(classes
  # Readers
  vtkACosmoReader
  vtkGenericIOUtilities
  vtkPCosmoReader
  vtkPGenericIOMultiBlockReader
  vtkPGenericIOMultiBlockWriter
  vtkPGenericIOReader
  vtkPMultiResolutionGenericIOReader

  # Filters
  vtkMinkowskiFilter
  vtkPANLHaloFinder
  vtkPANLSubhaloFinder
  vtkPLANLHaloFinder
  vtkPMergeConnected)

vtk_module_add_module(Visocyte::VTKExtensionsCosmoTools
  CLASSES ${classes})
vtk_module_include(Visocyte::VTKExtensionsCosmoTools
  PRIVATE
    ${GENERIC_IO_INCLUDE_DIR})
vtk_module_link(Visocyte::VTKExtensionsCosmoTools
  PRIVATE
    ${GENERIC_IO_LIBRARIES})

visocyte_add_server_manager_xmls(
  XMLS  ## CosmoTools Readers
        resources/AdaptiveCosmoReader.xml
        resources/CosmoReader.xml
        resources/GenericIOReader.xml
        resources/GenericIOWriter.xml
        resources/MultiResolutionGenericIOReader.xml

        ## CosmoTools Filters
        resources/ANLHaloFinder.xml
        resources/ANLSubHaloFinder.xml
        resources/LANLHaloFinder.xml
        resources/MergeConnected.xml
        resources/MinkowskiFilter.xml)
