set(classes
  vtkThickenLayeredCells)

vtk_module_add_module(ThickenLayeredCells::vtkThickenLayeredCellsFilters
  CLASSES ${classes})

visocyte_add_server_manager_xmls(
  XMLS  ThickenLayeredCells.xml)
