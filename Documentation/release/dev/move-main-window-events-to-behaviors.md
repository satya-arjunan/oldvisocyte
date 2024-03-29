# Move Main Window Event handling to Behavior Classes

Visocyte and Visocyte-derived applications have their own main window class
that inherits from QMainWindow. The standard Qt approach to intercepting
signals to the main window (e.g. "close application") is to reimplement
QMainWindow's event methods (e.g. closeEvent(QCloseEvent*)). This polymorphic
solution is not available to Visocyte-derived applications comprised of
plugins, however. To facilitate a plugin's ability to influence the behavior
of Visocyte's main window, we introduce pqMainWindowEventManager,
a manager that emits signals when the main window receivesevents. These
signals are connected to pqMainWindowEventBehavior, a behavior class for
performing tasks originally executed in VisocyteMainWindow (with the exception
of the splash screen).
