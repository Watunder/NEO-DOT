import sys
import os
import shutil
import subprocess
import multiprocessing
import glob

script_path = os.path.abspath(__file__)


def replace_word(file_name, target_str, replace_str):
    text = ""
    with open(file_name, "r") as file:
        text = file.read()

    text = text.replace(target_str, replace_str)

    with open(file_name, "w") as file:
        file.write(text)


os.chdir(os.path.dirname(script_path))

job_opt = " -j" + str(multiprocessing.cpu_count())

if "platform=windows" in sys.argv:
    shutil.copy2("criware/adx2le/pc/libs/x64/cri_ware_pcx64_le.dll", "bin/cri_ware_pcx64_le.dll")

    # subprocess.run("scons platform=windows bits=32 target=release" + job_opt, shell = True)
    subprocess.run("scons platform=windows bits=64 target=release" + job_opt, shell=True)

elif "platform=macos" in sys.argv:
    # shutil.copytree("criware/adx2le/macosx/libs", "godot-proj/addons/adxle/bin/macos", dirs_exist_ok=True)

    subprocess.run("scons platform=macos bits=64 target=release" + job_opt, shell=True)

elif "platform=android" in sys.argv:
    for arch in ["arm64-v8a", "armeabi-v7a", "x86_64"]:
        os.makedirs(f"bin/android/{arch}", exist_ok=True)
        shutil.copy2(
            f"criware/adx2le/android/libs/{arch}/libcri_ware_android_LE.so",
            f"bin/android/{arch}/libcri_ware_android_le.so",
        )

    subprocess.run("scons platform=android android_arch=armeabi-v7a target=release" + job_opt, shell=True)
    subprocess.run("scons platform=android android_arch=arm64-v8a target=release" + job_opt, shell=True)
    subprocess.run("scons platform=android android_arch=x86_64 target=release" + job_opt, shell=True)

elif "platform=ios" in sys.argv:
    # shutil.copytree("criware/adx2le/ios/libs", "godot-proj/addons/adxle/bin/ios", dirs_exist_ok=True)

    subprocess.run("scons platform=ios ios_arch=arm64 target=release" + job_opt, shell=True)
    subprocess.run("scons platform=ios ios_arch=x86_64 target=release" + job_opt, shell=True)

    subprocess.run("lipo -create bin/libadxle.ios.arm64.dylib -output bin/libadxle.ios.dylib", shell=True)
    subprocess.run("lipo -create bin/libadxle.ios.x86_64.dylib -output bin/libadxle.ios-sim.dylib", shell=True)
