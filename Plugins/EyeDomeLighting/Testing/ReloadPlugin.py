from __future__ import print_function
# we should be able to load in a plugin multiple times without issues,
# as long as it's the same plugin.
import sys
import visocyte.simple

visocyte.simple.LoadDistributedPlugin("EyeDomeLighting", True, globals())
print('loaded the first time successfully')
visocyte.simple.LoadDistributedPlugin("EyeDomeLighting", True, globals())
print('loaded the second time successfully')
