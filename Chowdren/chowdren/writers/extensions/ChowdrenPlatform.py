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

from chowdren.writers.events import (ConditionMethodWriter,
    ActionMethodWriter, ExpressionMethodWriter, make_table)

class ChowdrenPlatform(ObjectWriter):
    class_name = 'ChowdrenPlatform'
    static = True

    def write_init(self, writer):
        pass

class PlatformAction(ActionMethodWriter):
    has_object = False

actions = make_table(PlatformAction, {
    0 : 'platform_set_lightbar',
    1 : 'platform_reset_lightbar',
    2 : 'set_local',
    3 : 'platform_set_border_size',
    4 : 'platform_open_achievements'
})

conditions = make_table(ConditionMethodWriter,  {
    0 : 'platform_is_tv'
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return ChowdrenPlatform
