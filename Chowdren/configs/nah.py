from chowdren.writers.events.system import get_loop_index_name
import re
import os

def use_deferred_collisions(converter):
    return False

FORCE_PS4 = False

def init(converter):
    converter.add_define('CHOWDREN_IS_NAH')
    converter.add_define('CHOWDREN_QUICK_SCALE')
    converter.add_define('CHOWDREN_POINT_FILTER')
    converter.add_define('CHOWDREN_OBSTACLE_IMAGE')
    converter.add_define('CHOWDREN_TEXTURE_GC')
    converter.add_define('CHOWDREN_CACHE_INI')
    converter.add_define('CHOWDREN_PASTE_REMOVE')
    converter.add_define('CHOWDREN_ACTIVE_REPLACE_COLOR')
    converter.add_define('CHOWDREN_ACTIVE_LOAD_SINGLE')
    converter.add_define('CHOWDREN_ACTIVE_LOOPING_APPEARING')
    if converter.platform_name != 'android':
        converter.add_define('CHOWDREN_SPECIAL_POINT_FILTER')
    converter.add_define('CHOWDREN_PASTE_PRECEDENCE')
    converter.add_define('CHOWDREN_JOYSTICK2_CONTROLLER')
    converter.add_define('CHOWDREN_NO_BACKDROPS')
    converter.add_define('CHOWDREN_FORCE_X360')
    converter.add_define('CHOWDREN_PASTE_CACHE')
    # converter.add_define('CHOWDREN_PASTE_SMALL_CACHE')
    converter.add_define('CHOWDREN_DEFAULT_SCALE', 2)
    converter.add_define('CHOWDREN_FORCE_16_9')
    converter.add_define('CHOWDREN_STEAM_APPID', 274270)
    # converter.add_define('CHOWDREN_PASTE_BROADPHASE')
    converter.add_define('CHOWDREN_USE_STEAM_LANGUAGE')

    if converter.platform_name == 'android':
        converter.add_define('CHOWDREN_FORCE_FILTER')
        converter.add_define('CHOWDREN_FORCE_FILL')

    strings = converter.game.globalStrings.items
    if converter.platform_name == 'ps4':
        converter.add_define('CHOWDREN_PRELOAD_ALL')
        converter.add_define('CHOWDREN_IGNORE_ASPECT')
        strings[8] = 'PS4'
    elif converter.platform_name in ('generic', 'd3d'):
        strings[8] = 'Desktop'

    if FORCE_PS4:
        strings[8] = 'PS4'

    converter.add_define('CHOWDREN_SAVE_PATH', 'save')

def write_frame_pre(converter, writer):
    if converter.current_frame in ('SPLASH',):
        return
    handle = converter.get_handle_from_name(('CONTROLLER VALUES',))
    obj = converter.get_object(handle)
    writer.putlnc('FrameObject * controller_values = %s;', obj)
    writer.putlnc('if (controller_values != NULL) {')
    writer.indent()
    writer.putlnc('if (is_joystick_pressed_once(1, 7)) {')
    writer.indent()
    writer.putlnc('controller_values->alterables->values.set(8, 1);')
    writer.dedent()
    writer.putlnc('} else {')
    writer.indent()
    writer.putlnc('controller_values->alterables->values.set(8, 0);')
    writer.end_brace()
    writer.end_brace()

def get_loop_name(converter, parameter):
    name = converter.convert_parameter(parameter)
    if name == 'global_strings->get(0)':
        return ('generic_ai_update', 'SWAT', 'NINJA')
    return None

def get_dynamic_loop_index(converter, exp):
    if exp.getName() != 'GlobalString':
        return None
    return 'generic_ai_update'

def get_dynamic_loop_call_name(converter, parameter):
    name = converter.convert_parameter(parameter)
    if name == 'global_strings->get(0)':
        items = parameter.parent.parent.actions[0].items[1].loader.items
        name = converter.convert_static_expression(items)
        if name in converter.system_object.loops:
            return name
        return 'generic_ai_update'


def write_loop(converter, loop_name, event_writer, writer):
    if loop_name not in ('SWAT', 'NINJA'):
        return
    ai_name = get_loop_index_name('generic_ai_update')
    index_name = get_loop_index_name(loop_name)
    event_writer.group.add_member('int %s' % ai_name, 0)
    writer.putlnc('%s = %s;', ai_name, index_name)

def use_image_preload(converter):
    return converter.platform_name not in ('ps4',)

def use_image_flush(converter, frame):
    if converter.platform_name != 'android':
        return False
    return frame.name == 'GAME FRAME'

alterable_int_objects = [
    ('SCOREVARIABLES_', [9, 10]),
    ('GOALOBJECT_', [9]),
    ('NEWSCREW_', [1]),
    ('PAYPHONE_', [1]),
    ('Safe3_', [0])
]

def use_alterable_int(converter, expression):
    obj = expression.get_object()
    name = expression.converter.get_object_name(obj)
    for (check_name, alts) in alterable_int_objects:
        if not name.startswith(check_name):
            continue
        if alts is None:
            return True
        index = expression.data.loader.value
        return index in alts
    return False

def use_repeated_collisions(converter):
    return False

def is_controller(value):
    if not value.startswith('((Active*)get_instance(controller_'):
        return False
    if not value.endswith('_instances))->alterables->values.get(12)'):
        return False
    return True

def is_global_fixed(value):
    return value.startswith('global_values->get(15)')

def write_pre(converter, writer, group):
    for condition in group.conditions:
        if condition.data.getName() != 'CompareFixedValue':
            continue
        value = condition.convert_index(0)
        if not is_controller(value) and not is_global_fixed(value):
            continue
        break
    else:
        return

    condition.force_single = True
    group.conditions.remove(condition)
    if converter.is_triggered:
        group.conditions.insert(1, condition)
    else:
        group.conditions.insert(0, condition)

save_paths = set([
    'Profile.ini',
    '.\\Bin\\Profile.ini',
    '.\\Bin\\Profile.ini'
])

audio_re = re.compile(re.escape('\\audio\\'), re.IGNORECASE)
src_re = re.compile(re.escape('\\src\\'), re.IGNORECASE)
bad_start = 'C:\\MMF2\\Python\\Chowdren\\nah\\'

ps4_achievements = {
    'MAYOR': 'KEY TO THE CITY',
    'PRIMINISTER': 'HEAD OF STATE',
    'KING_OF_ENGLAND': "I'M THE KING",
    'MEGALORD': 'MEGALORD',
    'MYSTERY_DOOR': 'MYSTERY DOOR',
    'YIPPEE_KAI_AY': 'YIPPEE KAI YAY',
    'EASTERN_PROMISE': 'EASTERN PROMISE',
    'EXECUTIONER': 'EXECUTIONER',
    'MEGALORD': 'MEGALORD',
    'DEAD_EYE': 'DEAD EYE',
    'KILL_BOGDAN': 'OVERKILL',
    'KILL_UPGRAYDD': 'BARE CARDIO',
    'GET_THE_CHOPPA': 'GET THE CHOPPA!'
}

android_replacements = {
    'MAYOR': 'CgkIsM6Pt5cYEAIQAQ',
    'PRIMINISTER': 'CgkIsM6Pt5cYEAIQAg',
    'KING_OF_ENGLAND': 'CgkIsM6Pt5cYEAIQAw',
    'MEGALORD': 'CgkIsM6Pt5cYEAIQBA',
    'MYSTERY_DOOR': 'CgkIsM6Pt5cYEAIQBQ',
    'YIPPEE_KAI_AY': 'CgkIsM6Pt5cYEAIQBg',
    'EASTERN_PROMISE': 'CgkIsM6Pt5cYEAIQBw',
    'EXECUTIONER': 'CgkIsM6Pt5cYEAIQCA',
    'DEAD_EYE': 'CgkIsM6Pt5cYEAIQCQ',
    'KILL_BOGDAN': 'CgkIsM6Pt5cYEAIQCg',
    'KILL_UPGRAYDD': 'CgkIsM6Pt5cYEAIQCw',
    'GET_THE_CHOPPA': 'CgkIsM6Pt5cYEAIQDA'
}

ps4_list = [
    'MEGALORD',
    "I'M THE KING",
    'HEAD OF STATE',
    'KEY TO THE CITY',
    'GET THE CHOPPA!',
    'EXECUTIONER',
    'DEAD EYE',
    'BARE CARDIO',
    'OVERKILL',
    'EASTERN PROMISE',
    'YIPPEE KAI YAY',
    'MYSTERY DOOR'
]

ps4_replacements = {}
for k, v in ps4_achievements.iteritems():
    ps4_replacements[k] = str(ps4_list.index(v))

def get_string(converter, value):
    if value == 'XBOX':
        return 'X360'
    elif value == 'Bin\\Charactertypes.ini':
        return '.\\Bin\\Charactertypes.ini'
    value = value.replace('.INI', '.ini')
    if value == 'BLANK.ini':
        return './Bin/BLANK.ini'
    value = value.replace('.PNG', '.png')
    value = value.replace('.OGG', '.ogg')
    value = value.replace('.WAV', '.wav')
    if value.startswith('Audio\\'):
        value = '.\\' + value
    value = audio_re.sub(re.escape('\\Audio\\'), value)
    value = src_re.sub(re.escape('\\Src\\'), value)
    value = value.replace(bad_start, '.\\')
    if converter.platform_name == 'android':
        if value in save_paths:
            return './Profile.ini'
        android_value = android_replacements.get(value, None)
        if android_value is not None:
            print 'achievement:', value
            return android_value
    elif converter.platform_name == 'ps4':
        if value in save_paths:
            return './save/Profile.INI'
        ps4_value = ps4_replacements.get(value, None)
        if ps4_value is not None:
            return ps4_value
    elif converter.platform_name == 'xb1':
        if value in save_paths:
            return './save/Profile.INI'
    else:
        if value in save_paths:
            return './Bin/Profile.ini'
    return value

try:
    from configs.local import nah
except ImportError:
    pass

def get_locals(converter):
    return nah.local_dict

def get_audio_preloads(converter):
    path = os.path.join(os.path.dirname(__file__), 'nahpreloads.py')
    with open(path, 'rb') as fp:
        data = eval(fp.read())
    return data
