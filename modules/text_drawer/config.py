def can_build(env, platform):
    return env["builtin_freetype"] and env["builtin_harfbuzz"]


def configure(env):
    pass
