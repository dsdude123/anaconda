import sys
sys.path.append('..')

from chowdren.platforms.generic import GenericPlatform
from chowdren.platforms.d3d import D3DPlatform
from chowdren.platforms.android import AndroidPlatform

classes = {
    'generic': GenericPlatform,
    'd3d': D3DPlatform,
    'android': AndroidPlatform
}

try:
    from extra import classes as extra_classes
    classes.update(extra_classes)
except ImportError, e:
    print 'Could not import all platforms'
    pass
