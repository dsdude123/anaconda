# Copyright (c) Mathias Kaerlev 2012-2015.
#
# This file is part of Anaconda.
#
# Anaconda is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Anaconda is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Anaconda.  If not, see <http://www.gnu.org/licenses/>.

from chowdren.writers.objects import ObjectWriter
from chowdren.common import get_animation_name, to_c, make_color
from chowdren.writers.events import (ComparisonWriter, StaticActionWriter,
    StaticConditionWriter, StaticExpressionWriter, make_table,
    ActionMethodWriter)

class OpenURL(ObjectWriter):
    class_name = 'OpenURL'
    static = True

    def write_init(self, writer):
        pass

class OpenAction(ActionMethodWriter):
    has_object = False
    method = 'open_url'

actions = make_table(StaticActionWriter, {
    0 : OpenAction
})

conditions = make_table(StaticConditionWriter, {
})

expressions = make_table(StaticExpressionWriter, {
})

def get_object():
    return OpenURL
