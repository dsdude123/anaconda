def init(converter):
    converter.add_define('CHOWDREN_OBSTACLE_IMAGE')
    converter.add_define('CHOWDREN_QUICK_SCALE')
    converter.add_define('CHOWDREN_POINT_FILTER')
    converter.add_define('CHOWDREN_JOYSTICK2_CONTROLLER')
    converter.add_define('CHOWDREN_FORCE_TRANSPARENT')
    converter.add_define('CHOWDREN_16BIT_IMAGE')
    converter.add_define('CHOWDREN_FORCE_TEXT_LAYOUT')
    converter.add_define('CHOWDREN_TEXT_USE_UTF8')

def finish(converter):
    pass

def use_gwen(converter):
    return False

def use_subapp_frames(converter):
    return False

def init_container(converter, container):
    pass

def init_group(converter, group):
    pass

def init_frame(converter, frame):
    pass

def write_pre(converter, writer, group):
    pass

def write_frame_post(converter, writer):
    pass

def write_frame_pre(converter, writer):
    pass

def init_obj(converter, obj):
    pass

def get_scale_method(converter, obj):
    # True for linear, False for linear
    return None

def use_simple_or(converter):
    return False

def get_locals(converter):
    return {}

def use_iteration_index(converter):
    return True

def use_global_alterables(converter, obj):
    return True

def use_single_global_alterables(converter, obj):
    return True

def use_global_int(converter, expression):
    return False

def use_alterable_int(converter, expression):
    return False

def use_counter_int(converter, expression):
    return False

def use_safe_division(converter):
    return True

def use_transparency_shader_reset(converter):
    return False

def get_startup_instances(converter, instances):
    return instances

def use_safe_create(converter):
    return False

def use_global_instances(converter):
    return True

def use_update_filtering(converter):
    return False

def use_deferred_collisions(converter):
    return False

def use_repeated_collisions(converter):
    return False

def use_image_flush(converter, frame):
    return True

def use_image_preload(converter):
    return False

def use_frame_preload(converter):
    return converter.config.use_image_preload()

def add_defines(converter):
    pass

def get_frames(converter, game, frames):
    return frames

def get_depth(converter, layer):
    return None

def get_object_depth(converter, obj):
    return None

def use_loop_selection_clear(converter):
    return True

def get_loop_name(converter, parameter):
    return None

def get_loop_call_names(converter, name):
    return None

def get_dynamic_loop_call_name(converter, parameter):
    return None

def get_dynamic_loop_index(converter, exp):
    return None

def get_fonts(converter):
    return ('SegoeUI',)

def use_edit_obj(converter):
    return True

def reorder_foreach(converter):
    return False

def use_webp(converter):
    return False

def use_condition_expression_iterator(converter):
    return True

def use_blitter_callback(converter, obj):
    return False

def get_string(converter, value):
    return value

def init_array_set_value(converter, event_writer):
    pass

def get_missing_image(converter, image):
    raise NotImplementedError('invalid image: %s' % repr(image))

def get_images(converter):
    return {}

def get_audio_preloads(converter):
    return []

def get_wave_sound(converter, data):
    return None

def write_loop(converter, loop_name, event_writer, writer):
    pass

def prepare_loop_body(converter, loop_name, writer, groups):
    return None
