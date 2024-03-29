import visocyte


class ProgressObject():
    '''Progress reporter. Creates a dummy vtkCinemaExporter proxy to use it
    as the emitter of progress events.'''
    def __init__(self):
        self.proxy = visocyte.simple.servermanager.CreateProxy(
                "exporters", "CinemaExporter")
        self.obj = self.proxy.GetClientSideObject()

    def UpdateProgress(self, progress):
        ''' 'progress' is expected to be in the range [0, 1]. '''
        self.obj.InvokeEvent(visocyte.vtk.vtkCommand.ProgressEvent, progress)

    def StartEvent(self):
        self.obj.InvokeEvent(visocyte.vtk.vtkCommand.StartEvent)

    def EndEvent(self):
        self.obj.InvokeEvent(visocyte.vtk.vtkCommand.EndEvent)
