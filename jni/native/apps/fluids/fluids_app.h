#ifndef FLUID_MECHANICS_H
#define FLUID_MECHANICS_H

#include "global.h"

#include "native_app.h"

class FluidMechanics : public NativeApp
{
public:
	FluidMechanics(const InitParams& params);

	// (GL context)
	virtual void rebind();

	void setMatrices(const Matrix4& volumeMatrix, const Matrix4& stylusMatrix);

	bool loadDataSet(const std::string& fileName);
	bool loadVelocityDataSet(const std::string& fileName);

	void setTangoValues(double tx, double ty, double tz, double rx, double ry, double rz, double q);
    void setGyroValues(double rx, double ry, double rz, double q);

    std::string getData();

	void updateSurfacePreview();

	void releaseParticles();

	void buttonPressed();
	float buttonReleased();

	struct Settings;
	struct State;

private:
	// virtual void detectObjects(ARMarkerInfo* markerInfo, int markerNum);

	// (GL context)
	virtual void renderObjects();

	struct Impl;
	std::unique_ptr<Impl> impl;
};

// ======================================================================
// Settings

enum SliceType {
	SLICE_CAMERA = 0, SLICE_AXIS = 1, SLICE_STYLUS = 2
};

struct FluidMechanics::Settings : public NativeApp::Settings
{
	Settings()
	 : showVolume(true),
	   showSurface(false),
	   showStylus(true),
	   showSlice(false),
	   showCrossingLines(true),
	   sliceType(SLICE_STYLUS),
	   clipDist(defaultClipDist),
	   surfacePercentage(0.13), // XXX: hardcoded testing value
	   surfacePreview(false)
	{}

	static constexpr float defaultClipDist = 360.0f;

	bool showVolume, showSurface, showStylus, showSlice, showCrossingLines;
	SliceType sliceType;
	float clipDist; // if clipDist == 0, the clip plane is disabled
	double surfacePercentage;
	bool surfacePreview;

	void read(JNIEnv* env, jobject obj, jclass cls) const
	{
		SET_JNI_FIELD(obj, showVolume, Boolean, "Z", showVolume);
		SET_JNI_FIELD(obj, showSurface, Boolean, "Z", showSurface);
		SET_JNI_FIELD(obj, showStylus, Boolean, "Z", showStylus);
		SET_JNI_FIELD(obj, showSlice, Boolean, "Z", showSlice);
		SET_JNI_FIELD(obj, showCrossingLines, Boolean, "Z", showCrossingLines);
		SET_JNI_FIELD(obj, clipDist, Float, "F", clipDist);
		SET_JNI_FIELD(obj, surfacePreview, Boolean, "Z", surfacePreview);

		SET_JNI_FIELD(obj, sliceType, Int, "I", sliceType);

		SET_JNI_FIELD(obj, surfacePercentage, Double, "D", surfacePercentage);
	}

	void write(JNIEnv* env, jobject obj, jclass cls)
	{
		GET_JNI_FIELD(obj, showVolume, Boolean, "Z", showVolume);
		GET_JNI_FIELD(obj, showSurface, Boolean, "Z", showSurface);
		GET_JNI_FIELD(obj, showStylus, Boolean, "Z", showStylus);
		GET_JNI_FIELD(obj, showSlice, Boolean, "Z", showSlice);
		GET_JNI_FIELD(obj, showCrossingLines, Boolean, "Z", showCrossingLines);
		GET_JNI_FIELD(obj, clipDist, Float, "F", clipDist);
		GET_JNI_FIELD(obj, surfacePreview, Boolean, "Z", surfacePreview);

		int st;
		GET_JNI_FIELD(obj, sliceType, Int, "I", st);
		sliceType = static_cast<SliceType>(st);

		double value;
		GET_JNI_FIELD(obj, surfacePercentage, Double, "D", value);
		if (value < 0.0 || value > 1.0)
			throw std::runtime_error("Invalid percentage: " + Utility::toString(value));
		surfacePercentage = value;
	}
};

// ======================================================================
// State

enum ClipAxis {
	CLIP_NONE,
	CLIP_AXIS_X, CLIP_AXIS_Y, CLIP_AXIS_Z,
	CLIP_NEG_AXIS_X, CLIP_NEG_AXIS_Y, CLIP_NEG_AXIS_Z
};

struct FluidMechanics::State : public NativeApp::State
{
	State()
	 : tangibleVisible(true), stylusVisible(true),
	   computedZoomFactor(1.0f),
	   clipAxis(CLIP_NONE), lockedClipAxis(CLIP_NONE)
	{}

	const bool tangibleVisible;
	const bool stylusVisible;
	float computedZoomFactor;

	void read(JNIEnv* env, jobject obj, jclass cls) const
	{
		SET_JNI_FIELD(obj, tangibleVisible, Boolean, "Z", tangibleVisible);
		SET_JNI_FIELD(obj, stylusVisible, Boolean, "Z", stylusVisible);
		SET_JNI_FIELD(obj, computedZoomFactor, Float, "F", computedZoomFactor);
	}

	ClipAxis clipAxis;
	ClipAxis lockedClipAxis;
	Synchronized<Matrix4> modelMatrix;
	Synchronized<Matrix4> sliceModelMatrix;
	Synchronized<Matrix4> stylusModelMatrix;
};

#endif /* FLUID_MECHANICS_H */
