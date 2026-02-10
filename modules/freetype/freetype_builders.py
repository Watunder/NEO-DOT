"""Functions used to generate source files during build time

All such functions are invoked in a subprocess on Windows to prevent build flakiness.

"""

from platform_methods import subprocess_main
from compat import open_utf8, byte_to_str
import os


def make_builtin_vector_fonts(target, source, env):
    # Generate header file
    builtin_fonts_h_path = target[0]
    builtin_fonts_h = open_utf8(builtin_fonts_h_path, "w")
    builtin_fonts_h.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n")
    builtin_fonts_h.write("#ifndef _BUILTIN_VECTOR_FONTS_H\n")
    builtin_fonts_h.write("#define _BUILTIN_VECTOR_FONTS_H\n")

    # Generate cpp file
    builtin_fonts_cpp_path = target[1]
    builtin_fonts_cpp = open_utf8(builtin_fonts_cpp_path, "w")
    builtin_fonts_cpp.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n")
    builtin_fonts_cpp.write('#include "builtin_vector_fonts.gen.h"\n')

    # Process each font file
    # saving uncompressed, since freetype will reference from memory pointer
    for font_path in source:
        with open(font_path, "rb") as f:
            buf = f.read()

        name = os.path.splitext(os.path.basename(font_path))[0]

        builtin_fonts_h.write("extern const int _font_" + name + "_size;\n")
        builtin_fonts_h.write("extern const unsigned char _font_" + name + "[];\n")

        builtin_fonts_cpp.write("static const int _font_" + name + "_size = " + str(len(buf)) + ";\n")
        builtin_fonts_cpp.write("static const unsigned char _font_" + name + "[] = {\n")
        for j in range(len(buf)):
            builtin_fonts_cpp.write("\t" + byte_to_str(buf[j]) + ",\n")
        builtin_fonts_cpp.write("};\n")

    builtin_fonts_h.write("#endif")
    builtin_fonts_h.close()

    builtin_fonts_cpp.close()


if __name__ == "__main__":
    subprocess_main(globals())
