from chowdren.writers.objects import ObjectWriter

from chowdren.common import get_animation_name, to_c, make_color

from chowdren.writers.events import (StaticConditionWriter,
    StaticActionWriter, StaticExpressionWriter, make_table,
    ActionWriter, ActionMethodWriter, ConditionMethodWriter,
    ExpressionMethodWriter, EmptyAction)

class CommonDialog(ObjectWriter):
    class_name = 'DialogObject'
    static = True
    filename = 'dialogext'

    def write_init(self, writer):
        pass

actions = make_table(ActionMethodWriter, {
    0 : EmptyAction,
    11 : 'std::cout << ({1}) << std::endl'
})

actions.update(make_table(StaticActionWriter, {
    4 : 'set_default_filename',
    5 : 'set_default_directory'
}))

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return CommonDialog
