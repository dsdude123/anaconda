from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (StaticConditionWriter, 
    StaticActionWriter, StaticExpressionWriter, make_table)

class Util(ObjectWriter):
    class_name = 'Utility'
    static = True

    def write_init(self, writer):
        pass

actions = make_table(StaticActionWriter, {
})

conditions = make_table(StaticConditionWriter, {
})

expressions = make_table(StaticExpressionWriter, {
    6 : 'ModifyRange',
    2 : 'Limit'
})

def get_object():
    return Util