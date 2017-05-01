# we specify a root target for android to prevent all of the targets from spidering out
./build_ios/libdehaze.xcodeproj: libdehaze.gyp ./thirdparty/djinni/support-lib/support_lib.gyp dehaze.djinni
	sh ./run_djinni.sh
	thirdparty/gyp/gyp --depth=. -f xcode -DOS=ios --generator-output ./build_ios -Ithirdparty/djinni/common.gypi ./libdehaze.gyp
ios: ./build_ios/libdehaze.xcodeproj
	xcodebuild -project ios/Dehaze.xcodeproj \
           -scheme Dehaze \
           -configuration 'Debug' \
           -sdk iphonesimulator
# we specify a root target for android to prevent all of the targets from spidering out
GypAndroid.mk: libdehaze.gyp ./thirdparty/djinni/support-lib/support_lib.gyp dehaze.djinni
	sh ./run_djinni.sh
	ANDROID_BUILD_TOP=$(shell dirname `which ndk-build`) thirdparty/gyp/gyp --depth=. -f android -DOS=android -Ithirdparty/djinni/common.gypi ./libdehaze.gyp --root-target=libdehaze_jni
# this target implicitly depends on GypAndroid.mk since gradle will try to make it
android: GypAndroid.mk
cd android/ && ./gradlew app:assembleDebug
	@echo "Apks produced at:"
	@python thirdparty/djinni/example/glob.py ./ '*.apk'
