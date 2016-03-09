MAIN_ACTIVITY=fr.limsi.ARViewer/.$(shell grep "<activity android:name" AndroidManifest.xml | cut -d\" -f2)
SDK_PATH=/Users/lonnibesancon/Library/Android/sdk
NDK_PATH=/Users/lonnibesancon/Library/Android/android-ndk-r10e

APP_TAG=ARViewer
LOG_FILTER=$(APP_TAG):V NativeApp:V VES:V AndroidRuntime:E libEGL:W StrictMode:V libc:F DEBUG:I

all: compile-debug

compile-debug:
	+$(NDK_PATH)/ndk-build -j NDK_DEBUG=1
	cp thirdparty/TangoSDK_Gemma_Java.jar libs/	
	cp thirdparty/QCAR/libs/armeabi-v7a/libQCAR.so libs/armeabi-v7a/
	cp thirdparty/QCAR/libs/QCAR-hacked.jar libs/
	ant -Djava.compilerargs=-Xlint debug installd

# compile-release:
# 	LC_ALL= $(NDK_PATH)/ndk-build -j NDK_DEBUG=0
# 	ant release installr

# fast-compile-debug:
# 	@/bin/zsh -c 'echo > /tmp/.ant-input && echo > /tmp/.ant-output && \
# 		echo fast-compile-debug >> /tmp/.ant-input && \
# 		( while IFS='' read -r line; do echo "$$line"; \
# 		[[ "$$line" =~ "^ant>" ]] && exit; done \
# 		< <(tailf /tmp/.ant-output)) | strings'

clean:
	ant clean
	rm -rf obj libs

test:
	@urxvt -e sh -c "adb logcat -c; adb shell am start -n $(MAIN_ACTIVITY); adb logcat -s $(LOG_FILTER)"

debug-java:
	adb shell am start -e debug true -n $(MAIN_ACTIVITY)
	adb forward tcp:7777 jdwp:$$(adb jdwp | tail -1)
	jdb -sourcepath src -attach localhost:7777

debug-native:
	adb shell am start -e debug true -n $(MAIN_ACTIVITY)
	$(NDK_PATH)/ndk-gdb

log:
	adb logcat -s $(LOG_FILTER)
