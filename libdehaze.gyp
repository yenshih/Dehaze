{
    "targets": [
        {
            "target_name": "libdehaze_jni",
            "type": "shared_library",
            "dependencies": [
              "./thirdparty/djinni/support-lib/support_lib.gyp:djinni_jni",
            ],
            "ldflags": [ "-llog", "-Wl,--build-id,--gc-sections,--exclude-libs,ALL" ],
            "sources": [
              "./thirdparty/djinni/support-lib/jni/djinni_main.cpp",
              "<!@(python thirdparty/djinni/example/glob.py generated-src/jni   '*.cpp')",
              "<!@(python thirdparty/djinni/example/glob.py generated-src/cpp   '*.cpp')",
              "<!@(python thirdparty/djinni/example/glob.py src '*.cpp')",
            ],
            "include_dirs": [
              "generated-src/jni",
              "generated-src/cpp",
              "src/cpp",
            ],
        },
        {
            "target_name": "libdehaze_objc",
            "type": 'static_library',
            "dependencies": [
              "./thirdparty/djinni/support-lib/support_lib.gyp:djinni_objc",
            ],
            'direct_dependent_settings': {

            },
            "sources": [
              "<!@(python thirdparty/djinni/example/glob.py generated-src/objc  '*.cpp' '*.mm' '*.m')",
              "<!@(python thirdparty/djinni/example/glob.py generated-src/cpp   '*.cpp')",
              "<!@(python thirdparty/djinni/example/glob.py src '*.cpp')",
            ],
            "include_dirs": [
              "generated-src/objc",
              "generated-src/cpp",
              "src/cpp",
            ],
        },
    ],
}
