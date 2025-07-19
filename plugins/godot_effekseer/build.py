import sys
import os
import shutil
import subprocess
import multiprocessing

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
    subprocess.run("scons platform=windows bits=32 target=release" + job_opt, shell=True)
    subprocess.run("scons platform=windows bits=64 target=release" + job_opt, shell=True)

elif "platform=osx" in sys.argv:
    subprocess.run("scons platform=osx bits=64 target=release" + job_opt, shell=True)

elif "platform=android" in sys.argv:
    subprocess.run("scons platform=android android_arch=armv7 target=release" + job_opt, shell=True)
    subprocess.run("scons platform=android android_arch=arm64v8 target=release" + job_opt, shell=True)
    subprocess.run("scons platform=android android_arch=x86 target=release" + job_opt, shell=True)
    subprocess.run("scons platform=android android_arch=x86_64 target=release" + job_opt, shell=True)

elif "platform=ios" in sys.argv:
    subprocess.run("scons platform=ios ios_arch=armv7 target=release" + job_opt, shell=True)
    subprocess.run("scons platform=ios ios_arch=arm64 target=release" + job_opt, shell=True)
    subprocess.run("scons platform=ios ios_arch=x86_64 target=release" + job_opt, shell=True)

    subprocess.run(
        "lipo -create bin/libeffekseer.ios-armv7.dylib bin/libeffekseer.ios-arm64.dylib -output bin/libeffekseer.ios.dylib",
        shell=True,
    )
    subprocess.run("lipo -create bin/libeffekseer.ios-x86_64.dylib -output bin/libeffekseer.ios-sim.dylib", shell=True)

elif "platform=linux" in sys.argv:
    subprocess.run("scons platform=linux bits=32 target=release use_llvm=1" + job_opt, shell=True)
    subprocess.run("scons platform=linux bits=64 target=release use_llvm=1" + job_opt, shell=True)

elif "platform=html5" in sys.argv:
    subprocess.run("scons platform=html5 bits=32 target=release" + job_opt, shell=True)
