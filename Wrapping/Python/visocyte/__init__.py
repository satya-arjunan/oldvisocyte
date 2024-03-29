r"""
The visocyte package provides modules used to script Visocyte. Generally, users
should import the modules of interest directly e.g.::

  from visocyte.simple import *

However, one may want to import visocyte package before importing any of the
Visocyte modules to force backwards compatibility to an older version::

  # To run scripts written for Visocyte 4.0 in newer versions, you can use the
  # following.
  import visocyte
  visocyte.compatibility.major = 4
  visocyte.compatibility.minor = 0

  # Now, import the modules of interest.
  from visocyte.simple import *
"""

#==============================================================================
#
#  Program:   Visocyte
#  Module:    __init__.py
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.
#
#     This software is distributed WITHOUT ANY WARRANTY; without even
#     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#     PURPOSE.  See the above copyright notice for more information.
#
#==============================================================================

class _version(object):
    def __init__(self, major, minor):
      self.major = major
      self.minor = minor
    def GetVersion(self):
        """Return version as a float. Will return None is no version is
        specified."""
        if self.minor != None and self.major != None:
            version = float(self.minor)
            while version >= 1.0:
                version = version / 10.0
            version += float(self.major)
            return version
        return None
    def __lt__(self, other):
        """This will always return False if compatibility is not being forced
        to a particular version."""
        myversion = self.GetVersion()
        if not myversion:
            return False
        else:
            return myversion < other
    def __le__(self, other):
        """This will always return False if compatibility is not forced to a
        particular version."""
        myversion = self.GetVersion()
        if not myversion:
            return False
        else:
            return myversion <= other
    def __eq__(self, other):
        raise RuntimeError("Equal operation not supported.")
    def __ne__(self, other):
        raise RuntimeError("NotEqual operation not supported.")
    def __gt__(self, other):
        """This will always return True if compatibility is not being forced to
        a particular version"""
        myversion = self.GetVersion()
        if not myversion:
            return True
        else:
            return myversion > other
    def __ge__(self, other):
        """This will always return True if compatibility is not being forced to
        a particular version"""
        myversion = self.GetVersion()
        if not myversion:
            return True
        else:
            return myversion >= other
    def __repr__(self):
        myversion = self.GetVersion()
        if not myversion:
          return "(none)"
        return str(myversion)

class compatibility:
    """Class used to check version number and compatibility. Users should only
    set the compatibility explicitly to force backwards compatibility to and
    older versions.
    """
    minor = None
    major = None

    def GetVersion(cls):
        return _version(cls.major, cls.minor)
    GetVersion = classmethod(GetVersion)

# This is reimplemented in vtkSMCoreUtilities::SanitizeName(). Keep both
# implementations consistent.
def make_name_valid(name):
    """Make a string into a valid Python variable name."""
    if not name:
        return None
    import string
    valid_chars = "_%s%s" % (string.ascii_letters, string.digits)
    name = str().join([c for c in name if c in valid_chars])
    if name and not name[0].isalpha():
        name = 'a' + name
    return name

class options:
    """Values set here have any effect, only when importing the visocyte module
       in python interpretor directly i.e. not through pvpython or pvbatch. In
       that case, one should use command line arguments for the two
       executables"""

    """When True, act as pvbatch. Default behaviour is to act like pvpython"""
    batch = False

    """When True, acts like pvbatch --symmetric. Requires that batch is set to
    True to have any effect."""
    symmetric = False

    """When True, `visocyte.print_debug_info()` will result in printing the
    debug messages to stdout. Default is False, hence all debug messages will be
    suppressed."""
    print_debug_messages = False

    """When True, This mean the current process is a satelite and should not try to
    connect or do anything else."""
    satelite = False

def print_warning(text):
   """Print text"""
   print(text)

def print_error(text):
   """Print text"""
   print(text)

def print_debug_info(text):
   """Print text"""
   if options.print_debug_messages:
       print(text)

class NotSupportedException(Exception):
    """Exception that is fired when obsolete API is used in a script."""
    def __init__(self, msg):
        self.msg = msg
        print_debug_info("\nDEBUG: %s\n" % msg)

    def __str__(self):
        return "%s" % (self.msg)


"""This variable is set whenever Python is initialized within a Visocyte
Qt-based application. Modules within the 'visocyte' package often use this to
taylor their behaviour based on whether the Python environment is embedded
within an application or not."""
fromGUI = False
