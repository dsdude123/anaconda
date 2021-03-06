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

from chowdren.writers.events import (StaticConditionWriter, StaticActionWriter,
                                     StaticExpressionWriter, ExpressionWriter,
                                     make_table, EmptyAction)

class MessageBoxObject(ObjectWriter):
    class_name = 'DialogObject'
    filename = 'dialogext'
    static = True

    def write_init(self, writer):
        pass

actions = make_table(StaticActionWriter, {
    0 : 'set_title',
    1 : 'set_text',
    7 : 'set_ok',
    9 : 'set_yes_no',
    24 : 'set_modal',
    28 : 'set_topmost',
    35 : 'create'
})

class OnSuccess(StaticConditionWriter):
    method = 'is_success'
    is_always = True
    post_event = True

class OnFailure(StaticConditionWriter):
    method = 'is_failure'
    is_always = True
    post_event = True

conditions = make_table(StaticConditionWriter, {
    4 : OnFailure,
    5 : OnSuccess,
    8 : OnSuccess
})

expressions = make_table(StaticExpressionWriter, {
})

def get_object():
    return MessageBoxObject
