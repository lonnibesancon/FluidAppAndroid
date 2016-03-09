#ifndef NATIVE_APP_H
#define NATIVE_APP_H

#include "global.h"

// #include "AR/ar.h"

#include "apps/app.h"
#include "util/worker_thread.h"

class NativeApp
{
public:
	NativeApp(const InitParams& params);
	virtual ~NativeApp();

	// (GL context)
	virtual void rebind();

	// (GL context)
	virtual void reshape(unsigned int width, unsigned int height);

	// virtual void processVideoFrame(const unsigned char* buf);

	// (GL context)
	void render();

	struct Settings;
	struct State;

	typedef std::shared_ptr<Settings> SettingsPtr;
	typedef std::shared_ptr<State> StatePtr;

	SettingsPtr getSettings() const { return settings; }
	StatePtr getState() const { return state; }

protected:
	NativeApp(const InitParams& params, SettingsPtr settings_, StatePtr state_);

	// virtual void detectObjects(ARMarkerInfo* markerInfo, int markerNum) {}

	// (GL context)
	virtual void renderObjects() {}

	// void resetThreshold();

	unsigned int getScreenWidth() const { return screenWidth; }
	unsigned int getScreenHeight() const { return screenHeight; }

	// unsigned int getVideoWidth() const { return videoWidth; }
	// unsigned int getVideoHeight() const { return videoHeight; }

	Matrix4 getProjMatrix() const { return projMatrix; }
	Matrix4 getOrthoProjMatrix() const { return orthoProjMatrix; }

	float getNearClipDist() const { return projNearClipDist; }
	float getFarClipDist() const { return projFarClipDist; }

	// Compute the depth value at the given distance, in NDC coordinates
	// (i.e. in the frustum defined by projMatrix)
	float getDepthValue(float dist)
	{
		const float& near = projNearClipDist;
		const float& far = projFarClipDist;
		return (far+near)/(far-near) + (1/dist) * ((-2*far*near)/(far-near));
	}

	SettingsPtr settings;
	StatePtr state;

private:
	void init(const InitParams& params);

	unsigned int screenWidth, screenHeight;
	// unsigned int videoWidth, videoHeight;

	Matrix4 projMatrix, orthoProjMatrix;

	float projNearClipDist, projFarClipDist;

	// bool cameraFrameAvailable;
	// int threshold;

	// Surface2DPtr cameraSurface;
	// Synchronized<std::vector<unsigned char>> conversionBuffer;

	// void convertCameraFrame(const unsigned char* buf);
	// void detectMarkers(const unsigned char* buf);

	std::unique_ptr<WorkerThread<const unsigned char*>> conversionThread;
	std::unique_ptr<WorkerThread<const unsigned char*>> detectionThread;
};

#define SET_JNI_FIELD(obj, name, type, signature, value) \
	android_assert(cls); \
	jfieldID name##_fid = env->GetFieldID(cls, #name, signature); \
	android_assert(name##_fid); \
	env->Set##type##Field(obj, name##_fid, value);
	// TODO(?) check if the update was successful (with env->ExceptionOccurred())

#define GET_JNI_FIELD(obj, name, type, signature, value) \
	android_assert(cls); \
	jfieldID name##_fid = env->GetFieldID(cls, #name, signature); \
	android_assert(name##_fid); \
	value = env->Get##type##Field(obj, name##_fid);
	// TODO(?) check if the read was successful

// ======================================================================
// Settings

struct NativeApp::Settings
{
	Settings()
	 : zoomFactor(1.0f),
	   showCamera(true)
	{}

	virtual ~Settings() {}

	static constexpr float nativeZoomFactor = 2.0f; // global zoom multiplier

	float zoomFactor;
	bool showCamera;

	void read(JNIEnv* env, jobject obj, jclass cls) const
	{
		SET_JNI_FIELD(obj, showCamera, Boolean, "Z", showCamera);
		SET_JNI_FIELD(obj, zoomFactor, Float, "F", zoomFactor/nativeZoomFactor);
	}

	void write(JNIEnv* env, jobject obj, jclass cls)
	{
		GET_JNI_FIELD(obj, showCamera, Boolean, "Z", showCamera);

		float zf;
		GET_JNI_FIELD(obj, zoomFactor, Float, "F", zf);
		zoomFactor = zf*nativeZoomFactor;
	}
};

// ======================================================================
// State

struct NativeApp::State
{
	State() {}
	virtual ~State() {}
	void read(JNIEnv* env, jobject obj, jclass cls) const {}
};

#endif /* NATIVE_APP_H */
