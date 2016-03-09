#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <utility>
#include <stdexcept>

#include <android/log.h>

#define LOG_TAG "NativeApp"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#ifndef NDEBUG
	#define QUOTE2(x) #x
	#define QUOTE(x) QUOTE2(x)
	#define android_assert(cond) \
		if (!(cond)) { \
			throw std::logic_error( \
				"Assertion '" QUOTE(cond) "' failed in '" __FILE__ "' at line " QUOTE(__LINE__) "." \
			); \
		}
#else
	#define android_assert(cond)
#endif

#include <cstdlib>
#include <jni.h>
inline void throwJavaException(JNIEnv* env, const char* what)
{
	LOGD("Exception: %s", what);
	jclass c = env->FindClass("java/lang/RuntimeException");
	if (c != nullptr) {
		env->ThrowNew(c, what);
	} else {
		LOGE("java.lang.RuntimeException was not found!");
		std::abort();
	}
}

// #define DOUBLE_PRECISION
#include "util/linear_math.h"

#include "util/synchronized.h"
#include "util/utility.h"

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "fwd.h"

#endif /* GLOBAL_H */
