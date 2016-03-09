#include "native_app.h"

#include "apps/app.h"
#include "thirdparty/yuv2rgb.h"
#include "rendering/surface_2d.h"

#include <cstdlib>
#include <cstring>
#include <time.h>

#include "AR/ar.h"
#include "AR/param.h"
#include "AR/gsub.h"

// ======================================================================
// JNI interface

extern "C" {
	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_init(JNIEnv* env, jobject obj, jint appType, jstring baseDir); //, jint videoWidth, jint videoHeight);

	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_rebind(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_reshape(JNIEnv* env, jobject obj, jint width, jint height);

	// JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_processVideoFrame(JNIEnv* env, jobject obj, jbyteArray inArray);

	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_render(JNIEnv* env, jobject obj);

	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_getSettings(JNIEnv* env, jobject obj, jobject settingsObj);
	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_setSettings(JNIEnv* env, jobject obj, jobject settingsObj);

	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_getState(JNIEnv* env, jobject obj, jobject stateObj);
}

// (end of JNI interface)
// ======================================================================

NativeApp::NativeApp(const InitParams& params)
 : settings(new Settings), state(new State)
{
	init(params);
}

NativeApp::NativeApp(const InitParams& params,
	SettingsPtr settings_, StatePtr state_)
 : settings(settings_), state(state_)
{
	init(params);
}

void NativeApp::init(const InitParams& params)
{
	android_assert(settings);
	android_assert(state);

	// Default values
	screenWidth = 0;
	screenHeight = 0;
	// cameraFrameAvailable = false;
	// threshold = 100;

	// videoWidth = params.videoWidth;
	// videoHeight = params.videoHeight;
	// android_assert(videoWidth > 0 && videoHeight > 0);

	// cameraSurface.reset(new Surface2D(videoWidth, videoHeight));
	// conversionBuffer.resize(videoWidth*videoHeight*3);

	// // SIZE = 768, 432
	// // Distortion factors = 348,500000 245,000000 -22,000000 0,965188
	// // Intrinsic parameters =
	// // 768,59206 0,00000 383,50000 0,00000
	// // 0,00000 759,33359 195,00000 0,00000
	// // 0,00000 0,00000 1,00000 0,00000
	// //
	// // horizontal fov (degrees) = 2*arctan(768/(2*768.59206)) = 53.095°
	// // horizontal shift = 383.5/768-0.5 = ~0
	// // vertical shift = 195/432-0.5 = ~0
	// // radial distortion factor = -22/768 (???) = -0.02865 (usable in Blender "Lens Distortion" node)
	// // const std::string cameraParaFile = params.baseDir + "/tablet_camera_para.dat";
	//
	// ARParam wparam;
	// // if (arParamLoad(cameraParaFile.c_str(), 1, &wparam) < 0)
	// // 	throw std::runtime_error("Error loading camera parameters \"" + cameraParaFile + "\"");
	//
	// // Make sure the calibration parameters are the same in ARToolKit and Vuforia
	// // (values were obtained from QCAR::CameraCalibration)
	// wparam.xsize = videoWidth;
	// wparam.ysize = videoHeight;
	// // std::memset(wparam.dist_factor, 0, sizeof(wparam.dist_factor));
	// wparam.dist_factor[0] = videoWidth/2;
	// wparam.dist_factor[1] = videoHeight/2;
	// wparam.dist_factor[2] = 0; // according to QCAR::CameraCalibration
	// wparam.dist_factor[3] = 1; // no scaling
	// std::memset(wparam.mat, 0, sizeof(wparam.mat));
	// // wparam.mat[0][0] = 629.504028; // fx
	// wparam.mat[0][0] = 786.880005; // fx
	// wparam.mat[0][1] = 0; // s
	// // wparam.mat[1][1] = 629.504028; // fy
	// wparam.mat[1][1] = 786.880005; // fy
	// // wparam.mat[0][2] = 320; // cx => videoWidth/2
	// // wparam.mat[1][2] = 240; // cy => videoHeight/2
	// wparam.mat[0][2] = 400; // cx => videoWidth/2
	// wparam.mat[1][2] = 240; // cy => videoHeight/2
	// wparam.mat[2][2] = 1;
	//
	// ARParam cparam;
	// arParamChangeSize(&wparam, videoWidth, videoHeight, &cparam);
	// arInitCparam(&cparam);
	// argInit(&cparam, 1.0, 0, 0, 0, 0);
	//
	// // Retrieve the projection matrix
	// extern double gl_cpara[16]; // filled in by artoolkit/gsub.c
	// for (int i = 0; i < 16; ++i)
	// 	projMatrix.data_[i] = gl_cpara[i];
	// // LOGD("proj = %s", Utility::toString(projMatrix).c_str());

	// Create an orthographic projection matrix
	orthoProjMatrix = Matrix4::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f);
	// const float aspect = (float)videoWidth/videoHeight;
	// orthoProjMatrix = Matrix4::ortho(-1.0f, 1.0f, -1/aspect, 1/aspect, 1.0f, -1.0f);

	// // Start threads
	// using namespace std::placeholders;
	// conversionThread.reset(
	// 	new WorkerThread<const unsigned char*>(
	// 		std::bind(&NativeApp::convertCameraFrame, this, _1)));
	// detectionThread.reset(
	// 	new WorkerThread<const unsigned char*>(
	// 		std::bind(&NativeApp::detectMarkers, this, _1)));
}

NativeApp::~NativeApp()
{
	// ...
}

// (GL context)
void NativeApp::rebind()
{
	// cameraSurface->bind();
}

// (GL context)
void NativeApp::reshape(unsigned int width, unsigned int height)
{
	android_assert(width > 0 && height > 0);
	screenWidth = width;
	screenHeight = height;
	LOGD("screen size: %d x %d", screenWidth, screenHeight);

	// Create a perspective projection matrix
	projMatrix = Matrix4::perspective(35.0f, float(screenWidth)/screenHeight, 50.0f, 2500.0f);
	projMatrix[1][1] *= -1;
	projMatrix[2][2] *= -1;
	projMatrix[2][3] *= -1;

	projNearClipDist = -projMatrix[3][2] / (1+projMatrix[2][2]); // 50.0f
	projFarClipDist  =  projMatrix[3][2] / (1-projMatrix[2][2]); // 2500.0f

	glViewport(0, 0, screenWidth, screenHeight);

	// // Reverse pan&scan, since the screen is 16:9 and the scene (based
	// // on the 4:3 camera stream) is 4:3
	// const float aspect = (float)videoWidth/videoHeight;
	// const int sceneHeight = screenWidth / aspect;
	// glViewport(0, -(sceneHeight-(signed)screenHeight)/2, screenWidth, sceneHeight);
}

// void NativeApp::processVideoFrame(const unsigned char* buf)
// {
// 	if (settings->showCamera) {
// 		android_assert(detectionThread);
// 		android_assert(conversionThread);
//
// 		detectionThread->process(buf);
// 		conversionThread->process(buf);
//
// 		// XXX: workaround for a possible deadlock
// 		timespec start;
// 		clock_gettime(CLOCK_REALTIME, &start);
//
// 		while (!conversionThread->isWaiting() || !detectionThread->isWaiting()) {
// 			tthread::this_thread::yield();
//
// 			// XXX: workaround for a possible deadlock
// 			timespec now;
// 			clock_gettime(CLOCK_REALTIME, &now);
// 			if (now.tv_sec - start.tv_sec > 1) {
// 				LOGW("deadlock detected!");
// 				break;
// 			}
// 		}
//
// 	} else {
// 		detectMarkers(buf);
// 	}
// }

// void NativeApp::convertCameraFrame(const unsigned char* buf)
// {
// 	synchronized(conversionBuffer) {
// 		nv21_to_rgb_neon(conversionBuffer.data(), buf, videoWidth, videoHeight);
// 	}
// 	cameraFrameAvailable = true;
// }

// void NativeApp::detectMarkers(const unsigned char* buf)
// {
// 	ARMarkerInfo* markerInfo = nullptr;
// 	int markerNum = 0;
//
// 	// const int prevThresh = threshold;
//
// 	if (arDetectMarker(const_cast<ARUint8*>(buf), &threshold, &markerInfo, &markerNum) < 0) {
// 		// LOGE("processVideoFrame: arDetectMarker() failed");
// 		// return;
// 		throw std::runtime_error("arDetectMarker() failed");
// 	}
//
// 	// if (threshold != prevThresh)
// 	// 	LOGD("brightness threshold adjusted from %d to %d", prevThresh, threshold);
//
// 	// LOGD("detected %d markers", markerNum);
//
// 	detectObjects(markerInfo, markerNum);
// }

// void NativeApp::resetThreshold()
// {
// 	threshold = 100; // FIXME: hardcoded (and duplicated) value
// }

// (GL context)
void NativeApp::render()
{
	// if (cameraFrameAvailable) {
	// 	synchronized(conversionBuffer) {
	// 		cameraSurface->updateTexture(conversionBuffer.data());
	// 	}
	// 	cameraFrameAvailable = false;
	// }

	// glDepthMask(true);
	// glDepthFunc(GL_LEQUAL);
	//
	// if (settings->showCamera && cameraSurface->isTextureInitialized()) {
	// 	// Only clear the depth buffer explicitely, since the color buffer
	// 	// will be erased by the camera view
	// 	glClear(GL_DEPTH_BUFFER_BIT);
	// } else {
	// 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// }
	//
	// if (settings->showCamera) {
	// 	glDisable(GL_BLEND);
	// 	glDisable(GL_DEPTH_TEST);
	// 	glDepthMask(false);
	// 	cameraSurface->render(orthoProjMatrix);
	// }

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderObjects();
}

JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_init(JNIEnv* env,
	jobject obj, jint appType, jstring baseDir) //, jint videoWidth, jint videoHeight)
{
	try {
		// LOGD("(JNI) init()");

		if (App::getInstance())
			throw std::runtime_error("init() was already called");

		// if (videoWidth <= 0 || videoHeight <= 0) {
		// 	throw std::runtime_error("Invalid video size: "
		// 		"(" + Utility::toString(videoWidth) + "," + Utility::toString(videoHeight) + ")");
		// }

		const char* ptr = env->GetStringUTFChars(baseDir, nullptr);
		if (!ptr) throw std::runtime_error("GetStringUTFChars() returned null");
		const std::string baseDirStr(ptr);
		env->ReleaseStringUTFChars(baseDir, ptr);

		InitParams params;
		params.baseDir = baseDirStr;
		// params.videoWidth = videoWidth;
		// params.videoHeight = videoHeight;

		App::create(appType, params);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

// (GL context)
JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_rebind(JNIEnv* env, jobject obj)
{
	try {
		// LOGD("(JNI) rebind()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		// Forcefully rebind all GL objects, since the GL context may
		// have changed
		App::getInstance()->rebind();

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

// (GL context)
JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_reshape(JNIEnv* env,
	jobject obj, jint width, jint height)
{
	try {
		// LOGD("(JNI) reshape(%d, %d)", width, height);

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (width <= 0 || height <= 0) {
			throw std::runtime_error("Invalid screen size: "
				"(" + Utility::toString(width) + "," + Utility::toString(height) + ")");
		}

		App::getInstance()->reshape(width, height);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

// JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_processVideoFrame(JNIEnv* env,
// 	jobject obj, jbyteArray inArray)
// {
// 	try {
// 		// LOGD("(JNI) processVideoFrame()");
//
// 		if (!App::getInstance())
// 			throw std::runtime_error("init() was not called");
//
// 		jbyte* in = env->GetByteArrayElements(inArray, JNI_FALSE);
//
// 		if (!in)
// 			throw std::runtime_error("GetByteArrayElements() returned null");
//
// 		const unsigned char* buf = reinterpret_cast<const unsigned char*>(in);
// 		App::getInstance()->processVideoFrame(buf);
//
// 		env->ReleaseByteArrayElements(inArray, in, 0);
//
// 	} catch (const std::exception& e) {
// 		throwJavaException(env, e.what());
// 	}
// }

// (GL context)
JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_render(JNIEnv* env, jobject obj)
{
	try {
		// LOGD("(JNI) render()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		App::getInstance()->render();

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_getSettings(JNIEnv* env, jobject obj, jobject settingsObj)
{
	try {
		// LOGD("(JNI) getSettings()");

		if (!settingsObj)
			throw std::runtime_error("\"Settings\" object is null");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		jclass cls = env->GetObjectClass(settingsObj);
		if (!cls)
			throw std::runtime_error("GetObjectClass() returned null");

		NativeApp::Settings* settings = App::getInstance()->getSettings().get();
		android_assert(settings);
		settings->read(env, settingsObj, cls);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_setSettings(JNIEnv* env, jobject obj, jobject settingsObj)
{
	try {
		// LOGD("(JNI) setSettings()");

		if (!settingsObj)
			throw std::runtime_error("\"Settings\" object is null");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		jclass cls = env->GetObjectClass(settingsObj);
		if (!cls)
			throw std::runtime_error("GetObjectClass() returned null");

		NativeApp::Settings* settings = App::getInstance()->getSettings().get();
		android_assert(settings);
		settings->write(env, settingsObj, cls);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_NativeApp_getState(JNIEnv* env, jobject obj, jobject stateObj)
{
	try {
		// LOGD("(JNI) getState()");

		if (!stateObj)
			throw std::runtime_error("\"State\" object is null");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		jclass cls = env->GetObjectClass(stateObj);
		if (!cls)
			throw std::runtime_error("GetObjectClass() returned null");

		NativeApp::State* state = App::getInstance()->getState().get();
		android_assert(state);
		state->read(env, stateObj, cls);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}
