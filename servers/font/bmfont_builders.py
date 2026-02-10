"""Functions used to generate source files during build time

All such functions are invoked in a subprocess on Windows to prevent build flakiness.

"""

from platform_methods import subprocess_main
from compat import open_utf8, byte_to_str
import os
import re


def _parse_page_files_from_fnt(fnt_path):
    with open(fnt_path, "rb") as f:
        data = f.read()
    base_dir = os.path.dirname(os.path.abspath(fnt_path))
    if base_dir and not base_dir.endswith(os.sep):
        base_dir += os.sep

    try:
        text = data.decode("utf-8")
    except Exception:
        text = data.decode("utf-8", errors="replace")

    page_files = []

    # XML Format
    if "<" in text and "page" in text:
        for m in re.finditer(r'<page\s[^>]*file="([^"]+)"', text, re.IGNORECASE):
            page_files.append(os.path.join(base_dir, m.group(1)))

    # Text Format
    if not page_files:
        for line in text.splitlines():
            line = line.strip()
            if not line.startswith("page "):
                continue
            m = re.search(r'file="([^"]+)"', line)
            if m:
                page_files.append(os.path.join(base_dir, m.group(1)))

    return page_files


def make_builtin_bitmap_fonts(target, source, env):
    builtin_fonts_h_path = target[0]
    builtin_fonts_cpp_path = target[1]

    builtin_fonts_h = open_utf8(builtin_fonts_h_path, "w")
    builtin_fonts_h.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n")
    builtin_fonts_h.write("#ifndef _BUILTIN_BITMAP_FONTS_H\n")
    builtin_fonts_h.write("#define _BUILTIN_BITMAP_FONTS_H\n\n")

    builtin_fonts_cpp = open_utf8(builtin_fonts_cpp_path, "w")
    builtin_fonts_cpp.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n")
    builtin_fonts_cpp.write('#include "builtin_bitmap_fonts.gen.h"\n\n')

    # Process each font file
    for fnt_node in source:
        fnt_path = os.path.normpath(os.path.abspath(str(fnt_node)))
        if not fnt_path.lower().endswith(".fnt"):
            continue
        try:
            with open(fnt_path, "rb") as f:
                fnt_buf = f.read()
        except Exception:
            continue
        page_files = _parse_page_files_from_fnt(fnt_path)
        if not page_files:
            continue

        name = os.path.splitext(os.path.basename(fnt_path))[0]
        name = name.replace("-", "_").replace(".", "_")

        # Generate header file
        builtin_fonts_h.write("extern const int _font_" + name + "_fnt_size;\n")
        builtin_fonts_h.write("extern const unsigned char _font_" + name + "_fnt[];\n")
        builtin_fonts_h.write("extern const int _font_" + name + "_page_count;\n")
        builtin_fonts_h.write("extern const int _font_" + name + "_page_sizes[];\n")
        builtin_fonts_h.write("extern const unsigned char *const _font_" + name + "_page[];\n\n")

        # Generate cpp file
        builtin_fonts_cpp.write("static const int _font_" + name + "_fnt_size = " + str(len(fnt_buf)) + ";\n")
        builtin_fonts_cpp.write("static const unsigned char _font_" + name + "_fnt[] = {\n")
        for j in range(len(fnt_buf)):
            builtin_fonts_cpp.write("\t" + byte_to_str(fnt_buf[j]) + ",\n")
        builtin_fonts_cpp.write("};\n\n")

        page_sizes = []
        page_bufs = []
        for i, page_path in enumerate(page_files):
            try:
                with open(page_path, "rb") as pf:
                    page_buf = pf.read()
            except Exception:
                continue
            page_sizes.append(len(page_buf))
            page_bufs.append(page_buf)
            builtin_fonts_cpp.write(
                "static const int _font_" + name + "_page" + str(i) + "_size = " + str(len(page_buf)) + ";\n"
            )
            builtin_fonts_cpp.write("static const unsigned char _font_" + name + "_page" + str(i) + "[] = {\n")
            for j in range(len(page_buf)):
                builtin_fonts_cpp.write("\t" + byte_to_str(page_buf[j]) + ",\n")
            builtin_fonts_cpp.write("};\n\n")

        n = len(page_bufs)
        builtin_fonts_cpp.write("static const int _font_" + name + "_page_count = " + str(n) + ";\n")
        builtin_fonts_cpp.write("static const int _font_" + name + "_page_sizes[] = { ")
        builtin_fonts_cpp.write(", ".join("_font_" + name + "_page" + str(i) + "_size" for i in range(n)) + " };\n")
        builtin_fonts_cpp.write("static const unsigned char *const _font_" + name + "_page[] = { ")
        builtin_fonts_cpp.write(", ".join("_font_" + name + "_page" + str(i) for i in range(n)) + " };\n\n")

    builtin_fonts_h.write("#endif\n")
    builtin_fonts_h.close()
    builtin_fonts_cpp.close()


if __name__ == "__main__":
    subprocess_main(globals())
