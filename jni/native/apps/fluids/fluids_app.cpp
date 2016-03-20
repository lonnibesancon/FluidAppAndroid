#include "fluids_app.h"

#include "apps/app.h"

#include "vtk_output_window.h"
#include "vtk_error_observer.h"
#include "volume.h"
#include "volume3d.h"
#include "isosurface.h"
#include "slice.h"
#include "rendering/cube.h"
#include "tracking/multi_marker.h"
#include "tracking/multi_marker_objects.h"
#include "loaders/loader_obj.h"
#include "rendering/mesh.h"
#include "rendering/lines.h"

#include <array>
#include <time.h> 

#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkDataSetReader.h>
#include <vtkXMLImageDataReader.h>
#include <vtkImageData.h>
#include <vtkImageResize.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkProbeFilter.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Renderer.h>
#include <QCAR/VideoBackgroundConfig.h>
#include <QCAR/Trackable.h>
#include <QCAR/TrackableResult.h>
#include <QCAR/Tool.h>
#include <QCAR/Tracker.h>
#include <QCAR/TrackerManager.h>
#include <QCAR/ImageTracker.h>
#include <QCAR/CameraCalibration.h>
#include <QCAR/UpdateCallback.h>
#include <QCAR/DataSet.h>
#include "interactionMode.h"
// #include <QCAR/Image.h>

#define NEW_STYLUS_RENDER

// ======================================================================
// JNI interface

extern "C" {
	JNIEXPORT jboolean JNICALL Java_fr_limsi_ARViewer_FluidMechanics_loadDataset(JNIEnv* env, jobject obj, jstring filename);
	JNIEXPORT jboolean JNICALL Java_fr_limsi_ARViewer_FluidMechanics_loadVelocityDataset(JNIEnv* env, jobject obj, jstring filename);

	// JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_initQCAR(JNIEnv* env, jobject obj);

	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_releaseParticles(JNIEnv* env, jobject obj);

	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_buttonPressed(JNIEnv* env, jobject obj);
	JNIEXPORT jfloat JNICALL Java_fr_limsi_ARViewer_FluidMechanics_buttonReleased(JNIEnv* env, jobject obj);

	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_getSettings(JNIEnv* env, jobject obj, jobject settingsObj);
	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_setSettings(JNIEnv* env, jobject obj, jobject settingsObj);

	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_getState(JNIEnv* env, jobject obj, jobject stateObj);
	JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_setTangoValues(JNIEnv* env, jobject obj, jdouble tx, jdouble ty, jdouble tz, jdouble rx, jdouble ry, jdouble rz, jdouble q);
    JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_setGyroValues(JNIEnv* env, jobject obj, jdouble rx, jdouble ry, jdouble rz, jdouble q);
    JNIEXPORT jstring JNICALL Java_fr_limsi_ARViewer_FluidMechanics_getData(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_setInteractionMode(JNIEnv* env, jobject obj, jint mode);
    JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_updateFingerPositions(JNIEnv* env, jobject obj, jfloat x, jfloat y, jint fingerID);
    JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_addFinger(JNIEnv* env, jobject obj, jfloat x, jfloat y, jint fingerID);
    JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_removeFinger(JNIEnv* env, jobject obj, jint fingerID);
    JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_reset(JNIEnv* env, jobject obj);
}

// (end of JNI interface)
// ======================================================================

struct Particle
{
	Vector3 pos;
	bool valid;
	int delayMs, stallMs;
	timespec lastTime;
};

struct FluidMechanics::Impl
{
	Impl(const std::string& baseDir);

	bool loadDataSet(const std::string& fileName);
	bool loadVelocityDataSet(const std::string& fileName);

	// void initQCAR();

	template <typename T>
	vtkSmartPointer<vtkImageData> loadTypedDataSet(const std::string& fileName);

	// (GL context)
	void rebind();

	// void detectObjects(ARMarkerInfo* markerInfo, int markerNum);
	void setMatrices(const Matrix4& volumeMatrix, const Matrix4& stylusMatrix);
	void updateSlicePlanes();
	void updateMatrices();

	// (GL context)
	void renderObjects();

	void updateSurfacePreview();

	void buttonPressed();
	float buttonReleased();

	void releaseParticles();
	Vector3 particleJitter();
	void integrateParticleMotion(Particle& p);

	bool computeCameraClipPlane(Vector3& point, Vector3& normal);
	bool computeAxisClipPlane(Vector3& point, Vector3& normal);
	bool computeStylusClipPlane(Vector3& point, Vector3& normal);

	void setTangoValues(double tx, double ty, double tz, double rx, double ry, double rz, double q);
	void setGyroValues(double rx, double ry, double rz, double q);
	std::string getData();
	void setInteractionMode(int mode);
	void updateFingerPositions(float x, float y, int fingerID);
	void addFinger(float x, float y, int fingerID);
	void removeFinger(int fingerID);
	void computeFingerInteraction();
	bool computeSeedingPlacement();
	void reset();
	int getFingerPos(int fingerID);

	Vector3 posToDataCoords(const Vector3& pos); // "pos" is in eye coordinates
	Vector3 dataCoordsToPos(const Vector3& dataCoordsToPos);

	Quaternion currentSliceRot ;
	Vector3 currentSlicePos ;
	Quaternion currentDataRot ;
	Vector3 currentDataPos ;
	//Quaternion representing the orientation of the tablet no matter the interaction mode or constrains
	Quaternion currentTabRot ;

	Synchronized<std::vector<Vector3> > fingerPositions;
	Synchronized<std::vector<Vector3> > prevFingerPositions ;
	Vector2 initialVector ;
	Vector3 prevVec ;
	Synchronized<Vector3> seedingPoint ;
	float screenW = 1920 ;
	float screenH = 1104 ;


	bool tangoEnabled = false ;
	int interactionMode = dataTangible ;
	bool seedPointPlacement = false ;


	FluidMechanics* app;
	std::shared_ptr<FluidMechanics::Settings> settings;
	std::shared_ptr<FluidMechanics::State> state;

	Synchronized<MultiMarker> tangible;
	Synchronized<MultiMarker> stylus;

	CubePtr cube, axisCube;

	vtkSmartPointer<vtkImageData> data, dataLow;
	int dataDim[3];
	Vector3 dataSpacing;

	vtkSmartPointer<vtkImageData> velocityData;

	typedef LinearMath::Vector3<int> DataCoords;
	// std::array<Particle, 10> particles;
	// std::array<Particle, 50> particles;
	// std::array<Particle, 100> particles;
	Synchronized<std::array<Particle, 200>> particles;
	timespec particleStartTime;
	static constexpr float particleSpeed = 0.15f;
	// static constexpr int particleReleaseDuration = 500; // ms
	static constexpr int particleReleaseDuration = 700; // ms
	static constexpr int particleStallDuration = 1000; // ms

	// static constexpr float stylusEffectorDist = 20.0f;
	static constexpr float stylusEffectorDist = 24.0f;
	// static constexpr float stylusEffectorDist = 30.0f;

	Synchronized<VolumePtr> volume;
	// Synchronized<Volume3dPtr> volume;
	Synchronized<IsoSurfacePtr> isosurface, isosurfaceLow;
	Synchronized<SlicePtr> slice;
	Synchronized<CubePtr> outline;
	Vector3 slicePoint, sliceNormal;
	float sliceDepth;
	Synchronized<std::vector<Vector3>> slicePoints; // max size == 6

	MeshPtr particleSphere, cylinder;
	LinesPtr lines;

	// Matrix4 qcarProjMatrix;
	// QCAR::DataSet* dataSetStonesAndChips;
	// Synchronized<Matrix4> qcarModelMatrix; // XXX: test
	// bool qcarVisible; // XXX: test

	vtkSmartPointer<vtkProbeFilter> probeFilter;

	Synchronized<Vector3> effectorIntersection;
	// Vector3 effectorIntersectionNormal;
	bool effectorIntersectionValid;

	bool buttonIsPressed;
};

FluidMechanics::Impl::Impl(const std::string& baseDir)
 : currentSliceRot(Quaternion(Vector3::unitX(), M_PI)),
   currentSlicePos(Vector3::zero()),
   currentDataPos(Vector3::zero()),
   currentDataRot(Quaternion(Vector3::unitX(), M_PI)),
   prevVec(Vector3::zero()),
   currentTabRot(Quaternion(Vector3::unitX(), M_PI)),
   buttonIsPressed(false) 
{
	seedingPoint = Vector3(-1,-1,-1);
	cube.reset(new Cube);
	axisCube.reset(new Cube(true));
	particleSphere = LoaderOBJ::load(baseDir + "/sphere.obj");
	cylinder = LoaderOBJ::load(baseDir + "/cylinder.obj");
	lines.reset(new Lines);



	for (Particle& p : particles)
		p.valid = false;
}

void FluidMechanics::Impl::reset(){
	seedingPoint = Vector3(-1,-1,-1);
	currentSliceRot = Quaternion(Vector3::unitX(), M_PI);
	currentDataRot = Quaternion(Vector3::unitX(), M_PI);
	currentSlicePos = Vector3(0, 0, 400);
	currentDataPos = Vector3(0,0,400);
	buttonIsPressed = false ;

	for (Particle& p : particles)
		p.valid = false;

	setMatrices(Matrix4::makeTransform(Vector3(0, 0, 400)),Matrix4::makeTransform(Vector3(0, 0, 400)));
	
}

void FluidMechanics::Impl::rebind()
{
	LOGD("OpenGL version: %s", glGetString(GL_VERSION));
	LOGD("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	LOGD("OpenGL extensions: %s", glGetString(GL_EXTENSIONS));

	cube->bind();
	axisCube->bind();
	lines->bind();
	particleSphere->bind();
	cylinder->bind();

	synchronized_if(volume) { volume->bind(); }
	synchronized_if(isosurface) { isosurface->bind(); }
	synchronized_if(isosurfaceLow) { isosurfaceLow->bind(); }
	synchronized_if(slice) { slice->bind(); }
	synchronized_if(outline) { outline->bind(); }
}

template <typename T>
vtkSmartPointer<vtkImageData> FluidMechanics::Impl::loadTypedDataSet(const std::string& fileName)
{
	vtkNew<T> reader;

	LOGI("Loading file: %s...", fileName.c_str());
	reader->SetFileName(fileName.c_str());

	vtkNew<VTKErrorObserver> errorObserver;
	reader->AddObserver(vtkCommand::ErrorEvent, errorObserver.GetPointer());

	reader->Update();

	if (errorObserver->hasError()) {
		// TODO? Throw a different type of error to let Java code
		// display a helpful message to the user
		throw std::runtime_error("Error loading data: " + errorObserver->getErrorMessage());
	}

	vtkSmartPointer<vtkImageData> data = vtkSmartPointer<vtkImageData>::New();
	data->DeepCopy(reader->GetOutputDataObject(0));

	return data;
}

bool FluidMechanics::Impl::loadDataSet(const std::string& fileName)
{
	// // Unload mesh data
	// mesh.reset();

	synchronized (particles) {
		// Unload velocity data
		velocityData = nullptr;

		// Delete particles
		for (Particle& p : particles)
			p.valid = false;
	}

	VTKOutputWindow::install();

	const std::string ext = fileName.substr(fileName.find_last_of(".") + 1);

	if (ext == "vtk")
		data = loadTypedDataSet<vtkDataSetReader>(fileName);
	else if (ext == "vti")
		data = loadTypedDataSet<vtkXMLImageDataReader>(fileName);
	else
		throw std::runtime_error("Error loading data: unknown extension: \"" + ext + "\"");

	data->GetDimensions(dataDim);

	double spacing[3];
	data->GetSpacing(spacing);
	dataSpacing = Vector3(spacing[0], spacing[1], spacing[2]);

	// Compute a default zoom value according to the data dimensions
	// static const float nativeSize = 128.0f;
	static const float nativeSize = 110.0f;
	state->computedZoomFactor = nativeSize / std::max(dataSpacing.x*dataDim[0], std::max(dataSpacing.y*dataDim[1], dataSpacing.z*dataDim[2]));
	// FIXME: hardcoded value: 0.25 (minimum zoom level, see the
	// onTouch() handler in Java code)
	state->computedZoomFactor = std::max(state->computedZoomFactor, 0.25f);

	dataLow = vtkSmartPointer<vtkImageData>::New();
	vtkNew<vtkImageResize> resizeFilter;
	resizeFilter->SetInputData(data.GetPointer());
	resizeFilter->SetOutputDimensions(std::max(dataDim[0]/3, 1), std::max(dataDim[1]/3, 1), std::max(dataDim[2]/3, 1));
	resizeFilter->InterpolateOn();
	resizeFilter->Update();
	dataLow->DeepCopy(resizeFilter->GetOutput());

	probeFilter = vtkSmartPointer<vtkProbeFilter>::New();
	probeFilter->SetSourceData(data.GetPointer());

	synchronized(outline) {
		LOGD("creating outline...");
		outline.reset(new Cube(true));
		outline->setScale(Vector3(dataDim[0]/2, dataDim[1]/2, dataDim[2]/2) * dataSpacing);
	}

	synchronized(volume) {
		LOGD("creating volume...");
		volume.reset(new Volume(data));
		// volume.reset(new Volume3d(data));
		if (fileName.find("FTLE7.vtk") != std::string::npos) { // HACK
			// volume->setOpacity(0.25f);
			volume->setOpacity(0.15f);
		}
	}

	if (fileName.find("FTLE7.vtk") == std::string::npos) { // HACK
		synchronized(isosurface) {
			LOGD("creating isosurface...");
			isosurface.reset(new IsoSurface(data));
			isosurface->setPercentage(settings->surfacePercentage);
		}

		synchronized(isosurfaceLow) {
			LOGD("creating low-res isosurface...");
			isosurfaceLow.reset(new IsoSurface(dataLow, true));
			isosurfaceLow->setPercentage(settings->surfacePercentage);
		}
	} else {
		isosurface.reset();
		isosurfaceLow.reset();
	}

	synchronized(slice) {
		LOGD("creating slice...");
		slice.reset(new Slice(data));
	}

	return true;
}

bool FluidMechanics::Impl::loadVelocityDataSet(const std::string& fileName)
{
	if (!data)
		throw std::runtime_error("No dataset currently loaded");

	VTKOutputWindow::install();

	const std::string ext = fileName.substr(fileName.find_last_of(".") + 1);

	if (ext == "vtk")
		velocityData = loadTypedDataSet<vtkDataSetReader>(fileName);
	else if (ext == "vti")
		velocityData = loadTypedDataSet<vtkXMLImageDataReader>(fileName);
	else
		throw std::runtime_error("Error loading data: unknown extension: \"" + ext + "\"");

	int velocityDataDim[3];
	velocityData->GetDimensions(velocityDataDim);

	if (velocityDataDim[0] != dataDim[0]
	    || velocityDataDim[1] != dataDim[1]
	    || velocityDataDim[2] != dataDim[2])
	{
		throw std::runtime_error(
			"Dimensions do not match: "
			"vel: " + Utility::toString(velocityDataDim[0]) + "x" + Utility::toString(velocityDataDim[1]) + "x" + Utility::toString(velocityDataDim[2])
			+ ", data: " + Utility::toString(dataDim[0]) + "x" + Utility::toString(dataDim[1]) + "x" + Utility::toString(dataDim[2])
		);
	}

	int dim = velocityData->GetDataDimension();
	if (dim != 3)
		throw std::runtime_error("Velocity data is not 3D (dimension = " + Utility::toString(dim) + ")");

	if (!velocityData->GetPointData() || !velocityData->GetPointData()->GetVectors())
		throw std::runtime_error("Invalid velocity data: no vectors found");

	return true;
}

Vector3 FluidMechanics::Impl::posToDataCoords(const Vector3& pos)
{
	Vector3 result;

	synchronized(state->modelMatrix) {
		// Transform "pos" into object space
		result = state->modelMatrix.inverse() * pos;
	}

	// Compensate for the scale factor
	result *= 1/settings->zoomFactor;

	// The data origin is on the corner, not the center
	result += Vector3(dataDim[0]/2, dataDim[1]/2, dataDim[2]/2) * dataSpacing;

	return result;
}

Vector3 FluidMechanics::Impl::particleJitter()
{
	return Vector3(
		(float(std::rand()) / RAND_MAX),
		(float(std::rand()) / RAND_MAX),
		(float(std::rand()) / RAND_MAX)
	) * 1.0f;
	// ) * 0.5f;
}

void FluidMechanics::Impl::buttonPressed()
{
	tangoEnabled = true ;

	//buttonIsPressed = true;
}

float FluidMechanics::Impl::buttonReleased()
{
	tangoEnabled = false ;
	/*buttonIsPressed = false;

	settings->surfacePreview = false;
	try {
		updateSurfacePreview();
		return settings->surfacePercentage;
	} catch (const std::exception& e) {
		LOGD("Exception: %s", e.what());
		return 0.0f;
	}*/
	return 0 ;
}

void FluidMechanics::Impl::releaseParticles()
{
	if (!velocityData || !state->tangibleVisible || !state->stylusVisible || 
		interactionMode!=seedPointTangible || interactionMode!=seedPointTouch || 
		interactionMode != seedPointHybrid){

		LOGD("Cannot place Seed");
		seedPointPlacement = false ;
		return;
	}
		
	//LOGD("Conditions met to place particles");
	Matrix4 smm;
	synchronized (state->stylusModelMatrix) {
		smm = state->stylusModelMatrix;
	}
	//LOGD("Got stylus Model Matrix");
	//const float size = 0.5f * (stylusEffectorDist + std::max(dataSpacing.x*dataDim[0], std::max(dataSpacing.y*dataDim[1], dataSpacing.z*dataDim[2])));
	//Vector3 dataPos = posToDataCoords(smm * Matrix4::makeTransform(Vector3(-size, 0, 0)*settings->zoomFactor) * Vector3::zero());
	Vector3 dataPos = posToDataCoords(seedingPoint) ;
	if (dataPos.x < 0 || dataPos.y < 0 || dataPos.z < 0
	    || dataPos.x >= dataDim[0] || dataPos.y >= dataDim[1] || dataPos.z >= dataDim[2])
	{
		LOGD("outside bounds");
		seedPointPlacement = false ;
		return;
	}
	LOGD("Coords correct");
	DataCoords coords(dataPos.x, dataPos.y, dataPos.z);

	clock_gettime(CLOCK_REALTIME, &particleStartTime);

	int delay = 0;
	LOGD("Starting Particle Computation");
	synchronized (particles) {
		for (Particle& p : particles) {
			p.pos = Vector3(coords.x, coords.y, coords.z) + particleJitter();
			p.lastTime = particleStartTime;
			p.delayMs = delay;
			delay += particleReleaseDuration/particles.size();
			p.stallMs = 0;
			p.valid = true;
		}
	}
}

/*void FluidMechanics::Impl::releaseParticles()
{
	if (!velocityData || !state->tangibleVisible || !state->stylusVisible || interactionMode!=seedPoint){
		seedPointPlacement = false ;
		return;
	}
		

	Matrix4 smm;
	synchronized (state->stylusModelMatrix) {
		smm = state->stylusModelMatrix;
	}

	const float size = 0.5f * (stylusEffectorDist + std::max(dataSpacing.x*dataDim[0], std::max(dataSpacing.y*dataDim[1], dataSpacing.z*dataDim[2])));
	Vector3 dataPos = posToDataCoords(smm * Matrix4::makeTransform(Vector3(-size, 0, 0)*settings->zoomFactor) * Vector3::zero());

	if (dataPos.x < 0 || dataPos.y < 0 || dataPos.z < 0
	    || dataPos.x >= dataDim[0] || dataPos.y >= dataDim[1] || dataPos.z >= dataDim[2])
	{
		LOGD("outside bounds");
		seedPointPlacement = false ;
		return;
	}

	DataCoords coords(dataPos.x, dataPos.y, dataPos.z);
	// LOGD("coords = %s", Utility::toString(coords).c_str());

	// vtkDataArray* vectors = velocityData->GetPointData()->GetVectors();
	// double* v = vectors->GetTuple3(coords.z*(dataDim[0]*dataDim[1]) + coords.y*dataDim[0] + coords.x);
	// LOGD("v = %f, %f, %f", v[0], v[1], v[2]);

	// TODO(?)
	// vtkNew<vtkStreamLine> streamLine;
	// streamLine->SetInputData(velocityData);
	// // streamLine->SetStartPosition(coords.x, coords.y, coords.z);
	// streamLine->SetStartPosition(dataPos.x, dataPos.y, dataPos.z);
	// streamLine->SetMaximumPropagationTime(200);
	// streamLine->SetIntegrationStepLength(.2);
	// streamLine->SetStepLength(.001);
	// streamLine->SetNumberOfThreads(1);
	// streamLine->SetIntegrationDirectionToForward();
	// streamLine->Update();
	// vtkDataArray* vectors = streamLine->GetPointData()->GetVectors();
	// android_assert(vectors);
	// unsigned int num = vectors->GetNumberOfTuples();
	// LOGD("num = %d", num);
	// for (unsigned int i = 0; i < num; ++j) {
	// 	double* v = vectors->GetTuple3(i);
	// 	Vector3 pos(v[0], v[1], v[2]);
	// 	LOGD("pos = %s", Utility::toString(pos).c_str());
	// }

	clock_gettime(CLOCK_REALTIME, &particleStartTime);

	int delay = 0;
	synchronized (particles) {
		for (Particle& p : particles) {
			p.pos = Vector3(coords.x, coords.y, coords.z) + particleJitter();
			p.lastTime = particleStartTime;
			p.delayMs = delay;
			delay += particleReleaseDuration/particles.size();
			p.stallMs = 0;
			p.valid = true;
		}
	}
}*/

void FluidMechanics::Impl::integrateParticleMotion(Particle& p)
{
	if (!p.valid)
		return;

	// Pause particle motion when the data is not visible
	if (!state->tangibleVisible)
		return;

	timespec now;
	clock_gettime(CLOCK_REALTIME, &now);

	int elapsedMs = (now.tv_sec - p.lastTime.tv_sec) * 1000
		+ (now.tv_nsec - p.lastTime.tv_nsec) / 1000000;

	p.lastTime = now;

	if (p.delayMs > 0) {
		p.delayMs -= elapsedMs;
		if (p.delayMs < 0)
			elapsedMs = -p.delayMs;
		else
			return;
	}

	if (p.stallMs > 0) {
		p.stallMs -= elapsedMs;
		if (p.stallMs < 0)
			p.valid = false;
		return;
	}

	vtkDataArray* vectors = velocityData->GetPointData()->GetVectors();

	while (elapsedMs > 0) {
		--elapsedMs;

		DataCoords coords = DataCoords(p.pos.x, p.pos.y, p.pos.z);

		if (coords.x < 0 || coords.y < 0 || coords.z < 0
		    || coords.x >= dataDim[0] || coords.y >= dataDim[1] || coords.z >= dataDim[2])
		{
			// LOGD("particle moved outside bounds");
			p.valid = false;
			return;
		}

		double* v = vectors->GetTuple3(coords.z*(dataDim[0]*dataDim[1]) + coords.y*dataDim[0] + coords.x);
		// LOGD("v = %f, %f, %f", v[0], v[1], v[2]);

		// Vector3 vel(v[0], v[1], v[2]);
		Vector3 vel(v[1], v[0], v[2]); // XXX: workaround for a wrong data orientation

		if (!vel.isNull()) {
			p.pos += vel * particleSpeed;
		} else {
			// LOGD("particle stopped");
			p.stallMs = particleStallDuration;
			break;
		}
	}
}

bool FluidMechanics::Impl::computeCameraClipPlane(Vector3& point, Vector3& normal)
{
	// static const float weight = 0.3f;
	// static const float weight = 0.5f;
	static const float weight = 0.8f;
	static bool wasVisible = false;
	static Vector3 prevPos;

	if (!state->tangibleVisible) {
		wasVisible = false;
		return false;
	}

	Matrix4 slicingMatrix;
	// synchronized(modelMatrix) { // not needed since this thread is the only one to write to "modelMatrix"
	// Compute the inverse rotation matrix to render this
	// slicing plane
	slicingMatrix = Matrix4((app->getProjMatrix() * state->modelMatrix).inverse().get3x3Matrix());
	// }

	// Compute the slicing origin location in data coordinates:

	// Center of the screen (at depth "clipDist")
	Vector3 screenSpacePos = Vector3(0, 0, settings->clipDist);

	// Transform the position in object space
	Vector3 pos = state->modelMatrix.inverse() * screenSpacePos;

	// Transform the screen normal in object space
	Vector3 n = (state->modelMatrix.transpose().get3x3Matrix() * Vector3::unitZ()).normalized();

	// Filter "pos" using a weighted average, but only in the
	// "n" direction (the screen direction)
	// TODO: Kalman filter?
	if (wasVisible)
		pos += -n.project(pos) + n.project(pos*weight + prevPos*(1-weight));
	wasVisible = true;
	prevPos = pos;

	// Transform the position back in screen space
	screenSpacePos = state->modelMatrix * pos;

	// Store the computed depth
	sliceDepth = screenSpacePos.z;

	// Unproject the center of the screen (at the computed depth
	// "sliceDepth"), then convert the result into data coordinates
	Vector3 pt = app->getProjMatrix().inverse() * Vector3(0, 0, app->getDepthValue(sliceDepth));
	Vector3 dataCoords = posToDataCoords(pt);
	slicingMatrix.setPosition(dataCoords);

	synchronized(slice) {
		slice->setSlice(slicingMatrix, sliceDepth, settings->zoomFactor);
	}

	point = pt;
	normal = -Vector3::unitZ();

	return true;
}

bool FluidMechanics::Impl::computeAxisClipPlane(Vector3& point, Vector3& normal)
{
	if (state->tangibleVisible) {
		Matrix3 normalMatrix = state->modelMatrix.inverse().transpose().get3x3Matrix();
		float xDot = (normalMatrix*Vector3::unitX()).normalized().dot(Vector3::unitZ());
		float yDot = (normalMatrix*Vector3::unitY()).normalized().dot(Vector3::unitZ());
		float zDot = (normalMatrix*Vector3::unitZ()).normalized().dot(Vector3::unitZ());
		// Prevent back and forth changes between two axis (unless no
		// axis is defined yet)
		const float margin = (state->clipAxis != CLIP_NONE ? 0.1f : 0.0f);
		if (std::abs(xDot) > std::abs(yDot)+margin && std::abs(xDot) > std::abs(zDot)+margin) {
			state->clipAxis = (xDot < 0 ? CLIP_AXIS_X : CLIP_NEG_AXIS_X);
		} else if (std::abs(yDot) > std::abs(xDot)+margin && std::abs(yDot) > std::abs(zDot)+margin) {
			state->clipAxis = (yDot < 0 ? CLIP_AXIS_Y : CLIP_NEG_AXIS_Y);
		} else if (std::abs(zDot) > std::abs(xDot)+margin && std::abs(zDot) > std::abs(yDot)+margin) {
			state->clipAxis = (zDot < 0 ? CLIP_AXIS_Z : CLIP_NEG_AXIS_Z);
		}

		if (state->lockedClipAxis != CLIP_NONE) {
			Vector3 axis;
			ClipAxis neg;
			switch (state->lockedClipAxis) {
				case CLIP_AXIS_X: axis = Vector3::unitX(); neg = CLIP_NEG_AXIS_X; break;
				case CLIP_AXIS_Y: axis = Vector3::unitY(); neg = CLIP_NEG_AXIS_Y; break;
				case CLIP_AXIS_Z: axis = Vector3::unitZ(); neg = CLIP_NEG_AXIS_Z; break;
				case CLIP_NEG_AXIS_X: axis = -Vector3::unitX(); neg = CLIP_AXIS_X; break;
				case CLIP_NEG_AXIS_Y: axis = -Vector3::unitY(); neg = CLIP_AXIS_Y; break;
				case CLIP_NEG_AXIS_Z: axis = -Vector3::unitZ(); neg = CLIP_AXIS_Z; break;
				default: android_assert(false);
			}
			float dot = (normalMatrix*axis).normalized().dot(Vector3::unitZ());
			if (dot > 0)
				state->lockedClipAxis = neg;
		}

	} else {
		state->clipAxis = state->lockedClipAxis = CLIP_NONE;
	}

	const ClipAxis ca = (state->lockedClipAxis != CLIP_NONE ? state->lockedClipAxis : state->clipAxis);

	if (ca == CLIP_NONE)
		return false;

	Vector3 axis;
	Quaternion rot;
	switch (ca) {
		case CLIP_AXIS_X: axis = Vector3::unitX(); rot = Quaternion(Vector3::unitY(), -M_PI/2)*Quaternion(Vector3::unitZ(), M_PI); break;
		case CLIP_AXIS_Y: axis = Vector3::unitY(); rot = Quaternion(Vector3::unitX(),  M_PI/2)*Quaternion(Vector3::unitZ(), M_PI); break;
		case CLIP_AXIS_Z: axis = Vector3::unitZ(); rot = Quaternion::identity(); break;
		case CLIP_NEG_AXIS_X: axis = -Vector3::unitX(); rot = Quaternion(Vector3::unitY(),  M_PI/2)*Quaternion(Vector3::unitZ(), M_PI); break;
		case CLIP_NEG_AXIS_Y: axis = -Vector3::unitY(); rot = Quaternion(Vector3::unitX(), -M_PI/2)*Quaternion(Vector3::unitZ(), M_PI); break;
		case CLIP_NEG_AXIS_Z: axis = -Vector3::unitZ(); rot = Quaternion(Vector3::unitX(),  M_PI); break;
		default: android_assert(false);
	}

	// Project "pt" on the chosen axis in object space
	Vector3 pt = state->modelMatrix.inverse() * app->getProjMatrix().inverse() * Vector3(0, 0, app->getDepthValue(settings->clipDist));
	Vector3 absAxis = Vector3(std::abs(axis.x), std::abs(axis.y), std::abs(axis.z));
	Vector3 pt2 = absAxis * absAxis.dot(pt);

	// Return to eye space
	pt2 = state->modelMatrix * pt2;

	Vector3 dataCoords = posToDataCoords(pt2);

	// static const float size = 128.0f;
	const float size = 0.5f * std::max(dataSpacing.x*dataDim[0], std::max(dataSpacing.y*dataDim[1], dataSpacing.z*dataDim[2]));

	Matrix4 proj = app->getProjMatrix(); proj[0][0] = -proj[1][1] / 1.0f; // same as "projMatrix", but with aspect = 1
	Matrix4 slicingMatrix = Matrix4((proj * Matrix4::makeTransform(dataCoords, rot)).inverse().get3x3Matrix());
	slicingMatrix.setPosition(dataCoords);
	synchronized(slice) {
		slice->setSlice(slicingMatrix, -proj[1][1]*size*settings->zoomFactor, settings->zoomFactor);
	}

	synchronized(state->sliceModelMatrix) {
		state->sliceModelMatrix = Matrix4(state->modelMatrix * Matrix4::makeTransform(state->modelMatrix.inverse() * pt2, rot, settings->zoomFactor*Vector3(size, size, 0.0f)));
	}

	if (!slice->isEmpty())
		state->lockedClipAxis = ca;
	else
		state->lockedClipAxis = CLIP_NONE;

	point = pt2;
	normal = state->modelMatrix.inverse().transpose().get3x3Matrix() * axis;

	return true;
}

// From: "Jittering Reduction in Marker-Based Augmented Reality Systems"
Matrix4 filter(const Matrix4& in, const Matrix4& prev, float posWeight, float rotWeight)
{
	// TODO: Kalman filter for position?

	Matrix4 result;

	for (unsigned int col = 0; col < 4; ++col) {
		for (unsigned int row = 0; row < 4; ++row) {
			if (row == 3) {
				// The last row is left unchanged
				result[col][row] = in[col][row];

			} else if (row == 0 && col < 3) {
				// Skip the first axis (side vector)
				continue;

			} else if (col < 3) { // orientation
				// Average the last 1/rotWeight values
				result[col][row] = in[col][row]*rotWeight + prev[col][row]*(1-rotWeight);

			} else { // position
				// Average the last 1/posWeight values
				result[col][row] = in[col][row]*posWeight + prev[col][row]*(1-posWeight);
			}
		}
	}

	Vector3 forward(result[0][2], result[1][2], result[2][2]);
	forward.normalize();
	result[0][2] = forward.x;
	result[1][2] = forward.y;
	result[2][2] = forward.z;

	Vector3 up(result[0][1], result[1][1], result[2][1]);
	up.normalize();

	// Recompute the side vector, then the up vector, to make sure the
	// coordinate system remains orthogonal

	Vector3 side = forward.cross(-up);
	side.normalize();
	result[0][0] = side.x;
	result[1][0] = side.y;
	result[2][0] = side.z;

	up = forward.cross(side);
	up.normalize();
	result[0][1] = up.x;
	result[1][1] = up.y;
	result[2][1] = up.z;

	return result;
}

bool FluidMechanics::Impl::computeStylusClipPlane(Vector3& point, Vector3& normal)
{
#if 0
	// static const float posWeight = 0.7f;
	// static const float rotWeight = 0.8f;
	static const float posWeight = 0.8f;
	static const float rotWeight = 0.8f;

	static bool wasVisible = false;
	static Matrix4 prevMatrix;

	if (!state->stylusVisible) {
		wasVisible = false;
		return false;
	}
#else
	if (!state->stylusVisible)
		return false;
#endif

	// FIXME: state->stylusModelMatrix may be invalid (non-invertible) in some cases
	try {

	// Vector3 pt = state->stylusModelMatrix * Vector3::zero();
	// LOGD("normal = %s", Utility::toString(normal).c_str());
	// LOGD("pt = %s", Utility::toString(pt).c_str());

	// static const float size = 128.0f;
	// static const float size = 180.0f;
	const float size = 0.5f * (60.0f + std::max(dataSpacing.x*dataDim[0], std::max(dataSpacing.y*dataDim[1], dataSpacing.z*dataDim[2])));

	Matrix4 planeMatrix = state->stylusModelMatrix;

#if 0
	if (wasVisible)
		planeMatrix = filter(planeMatrix, prevMatrix, posWeight, rotWeight);
	prevMatrix = planeMatrix;
	wasVisible = true;
#endif

	// Matrix4 planeMatrix = state->stylusModelMatrix;
	// // planeMatrix = planeMatrix * Matrix4::makeTransform(Vector3(-size, 0, 0)*settings->zoomFactor);

	// Project the stylus->data vector onto the stylus X axis
	Vector3 dataPosInStylusSpace = state->stylusModelMatrix.inverse() * state->modelMatrix * Vector3::zero();

	// Shift the clip plane along the stylus X axis in order to
	// reach the data, even if the stylus is far away
	// Vector3 offset = (-Vector3::unitX()).project(dataPosInStylusSpace);
#if 0
	// Shift the clip plane along the other axis in order to keep
	// it centered on the data
	Vector3 n = Vector3::unitZ(); // plane normal in stylus space
	// Vector3 v = dataPosInStylusSpace - offset; // vector from the temporary position to the data center point
	// offset += v.planeProject(n); // project "v" on the plane, and shift the clip plane according to the result
	Vector3 v = dataPosInStylusSpace;
	Vector3 offset = v.projectOnPlane(n); // project "v" on the plane, and shift the clip plane according to the result

	// Apply the computed offset
	planeMatrix = planeMatrix * Matrix4::makeTransform(offset);
#endif 
	// The slice will be rendered from the viewpoint of the plane
	Matrix4 proj = app->getProjMatrix(); proj[0][0] = -proj[1][1] / 1.0f; // same as "projMatrix", but with aspect = 1
	Matrix4 slicingMatrix = Matrix4((proj * planeMatrix.inverse() * state->modelMatrix).inverse().get3x3Matrix());

	Vector3 pt2 = planeMatrix * Vector3::zero();

	// Position of the stylus tip, in data coordinates
	Vector3 dataCoords = posToDataCoords(pt2);
	// LOGD("dataCoords = %s", Utility::toString(dataCoords).c_str());
	slicingMatrix.setPosition(dataCoords);

	synchronized(slice) {
		slice->setSlice(slicingMatrix, -proj[1][1]*size*settings->zoomFactor, settings->zoomFactor);
	}

	synchronized(state->sliceModelMatrix) {
		state->sliceModelMatrix = Matrix4(planeMatrix * Matrix4::makeTransform(Vector3::zero(), Quaternion::identity(), settings->zoomFactor*Vector3(size, size, 0.0f)));
	}

	point = pt2;
	normal = state->stylusModelMatrix.inverse().transpose().get3x3Matrix() * Vector3::unitZ();

	} catch (const std::exception& e) { LOGD("%s", e.what()); return false; }

	return true;
}

Vector3 FluidMechanics::Impl::dataCoordsToPos(const Vector3& dataCoords)
{
	Vector3 result = dataCoords;

	// The data origin is on the corner, not the center
	result -= Vector3(dataDim[0]/2, dataDim[1]/2, dataDim[2]/2) * dataSpacing;

	// Compensate for the scale factor
	result *= settings->zoomFactor;

	synchronized(state->modelMatrix) {
		// Transform "result" into eye space
		result = state->modelMatrix * result;
	}

	return result;
}

template <typename T>
T lowPassFilter(const T& cur, const T& prev, float alpha)
{ return prev + alpha * (cur-prev); }

void FluidMechanics::Impl::setMatrices(const Matrix4& volumeMatrix, const Matrix4& stylusMatrix)
{
	synchronized(state->modelMatrix) {
		state->modelMatrix = volumeMatrix;
	}

	synchronized(state->stylusModelMatrix) {
		state->stylusModelMatrix = stylusMatrix;
	}

	updateSlicePlanes();
}

void FluidMechanics::Impl::setInteractionMode(int mode){
	this->interactionMode = mode ;
}

void FluidMechanics::Impl::setTangoValues(double tx, double ty, double tz, double rx, double ry, double rz, double q){
	if(!data ){
		return ;
	}

	Vector3 vec(tx,ty,tz);

	if(tangoEnabled){
		//LOGD("Tango Enabled");
		Quaternion quat(rx,ry,rz,q);

		//LOGD("autoConstraint == %d",settings->autoConstraint);
		if(settings->autoConstraint){
			/*- n = normale du plan
			- v1 = ramener n dans le repère écran
			- v2 = ramener le déplacement de la tablette dans le repère écran
			- l = v2.length()
			- d = v1.normalized().dot(v2.normalized())
			- position du plan += n * l*d*/

			Vector3 trans = quat.inverse() * (vec-prevVec);
			float l = trans.length();
			float d = sliceNormal.normalized().dot(trans.normalized());
			trans = sliceNormal*l*d ;
			trans *= 300 ;
			trans *= -1 ;
			trans *= settings->precision ;
			trans.x *= settings->considerX * settings->considerTranslation ;
			trans.y *= settings->considerY * settings->considerTranslation ;
			trans.z *= settings->considerZ * settings->considerTranslation ;

			printAny(sliceNormal,"SliceNormal = ");
			printAny(trans,"Trans = ");
			currentSlicePos += trans ;
			//LOGD("D = %f  --  L = %f",d,l);
			printAny(trans, "Trans: ");
		}
		
		else{
			//Normal interaction
			Vector3 trans = quat.inverse() * (vec-prevVec);
			trans *= Vector3(1,-1,-1);	//Tango... -_-"
			trans *= 300 ;
			//trans.z *= -1 ;
			trans *= settings->precision ;

			//To constrain interaction
			trans.x *= settings->considerX * settings->considerTranslation ;
			trans.y *= settings->considerY * settings->considerTranslation ;
			trans.z *= settings->considerZ * settings->considerTranslation ;

			if(interactionMode == planeTangible || (interactionMode == seedPointTangible && settings->dataORplane == 1) || interactionMode == seedPointHybrid || interactionMode == dataPlaneHybrid || (interactionMode == dataPlaneTangible && settings->dataORplane == 1)){
				//currentSlicePos += trans ;	Version with the plane moving freely in the world
				currentSlicePos += trans ; 	//Version with a fix plane
			}
			else if(interactionMode == dataTangible || (interactionMode == seedPointTangible && settings->dataORplane == 0) || 
				    interactionMode == dataHybrid || (interactionMode == dataPlaneTangible && settings->dataORplane == 0)){
				currentDataPos +=trans ;
			}
			
			//updateMatrices();
		}
	}
	prevVec = vec ;

}

void FluidMechanics::Impl::setGyroValues(double rx, double ry, double rz, double q){
	if(!data){
		return ;
	}

	//Now we update the rendering according to constraints and interaction mode

	rz *=settings->precision * settings->considerZ * settings->considerRotation;
	ry *=settings->precision * settings->considerY * settings->considerRotation;
	rx *=settings->precision * settings->considerX * settings->considerRotation;
	//LOGD("Current Rot = %s", Utility::toString(currentSliceRot).c_str());
	if(tangoEnabled){
		if(interactionMode == planeTangible || (interactionMode == seedPointTangible && settings->dataORplane == 1) || interactionMode == seedPointHybrid || interactionMode == dataPlaneHybrid || (interactionMode == dataPlaneTangible && settings->dataORplane == 1)){
			Quaternion rot = currentSliceRot;
			rot = rot * Quaternion(rot.inverse() * (-Vector3::unitZ()), rz);
			rot = rot * Quaternion(rot.inverse() * -Vector3::unitY(), ry);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitX(), rx);
			//currentSliceRot = rot ; //Version with the plane moving freely in the world
			currentSliceRot = rot ;
		}
		else if(interactionMode == dataTangible || (interactionMode == seedPointTangible && settings->dataORplane == 0) || interactionMode == dataPlaneHybrid || (interactionMode == dataPlaneTangible && settings->dataORplane == 0)){
			Quaternion rot = currentDataRot;
			rot = rot * Quaternion(rot.inverse() * (-Vector3::unitZ()), rz);
			rot = rot * Quaternion(rot.inverse() * -Vector3::unitY(), ry);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitX(), rx);
			currentDataRot = rot;
		}

		//Now for the automatic constraining of interaction

	}
	
}
//Code adapted from 
//http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection
bool intersectPlane(const Vector3& n, const Vector3& p0, const Vector3& l0, const Vector3& l, float& t) 
{ 
	// Here we consider that if the normal in the direction of the screen, there is no intersection
    // assuming vectors are all normalized
    float denom = n.dot(l);//dotProduct(n, l); 
    //LOGD("DENOM = %f", denom);
    if (denom < 1e-6) { 
        Vector3 p0l0 = p0 - l0; 
        t = p0l0.dot(n)/denom ;
        //printAny(p0l0, "POLO = ");
        //printAny(p0l0.dot(n), "p0l0.dot(n) = ");
        //LOGD("t == %f", t);
        return (t >= 0); 
    } 
 
    return false; 
} 

//Returns true if the seeding is successful
bool FluidMechanics::Impl::computeSeedingPlacement(){
	Vector2 currentPos ;
	synchronized(fingerPositions){
		currentPos = Vector2(fingerPositions[0].x,fingerPositions[0].y);
	}

	//LOGD("Current Pos = %f --  %f",currentPos.x, currentPos.y);

	//Put screen coordinate between -1 and 1, and inverse Y to correspond to OpenGL conventions
	currentPos.x = (currentPos.x * 2 /screenW)-1 ;
	currentPos.y = (currentPos.y * 2 /screenH)-1 ;
	currentPos.y *= -1 ;

	//LOGD("Current Pos -1/1 = %f --  %f",currentPos.x, currentPos.y);

	//Get the world cornidates of the finger postion accoriding to the depth of the plane
	Vector3 ray(currentPos.x, currentPos.y, 1);
	ray = app->getProjMatrix().inverse() * ray ;
	ray.normalize();
	//printAny(ray, "RAY == ");
	
	float t ;
	//printAny(sliceNormal, "SLice NOrmal =");
	//printAny(slicePoint, "Slice Point = ");

	bool success = intersectPlane(sliceNormal, slicePoint, Vector3::zero(),ray, t) ;

	if(success){
		//LOGD("SUCCESS");
		//printAny(ray*t, "RAY * T = ");
		seedingPoint = ray*t ;
		releaseParticles();
		return true ;
	}
	else{
		//LOGD("FAIL");
		return false ;
		
	}

}

void FluidMechanics::Impl::computeFingerInteraction(){
	Vector2 currentPos ;
	Vector2 prevPos ;

	//Particle seeding case
	//LOGD("ComputeFingerInteraction Function");
	//LOGD("%d == %d   ---  %d", interactionMode, seedPoint, fingerPositions.size());
	if( (interactionMode == seedPointTangible ||interactionMode == seedPointHybrid || interactionMode==seedPointTouch) 
	   && fingerPositions.size() == 1 && settings->isSeeding == true){
		//LOGD("Seeding");
		if(computeSeedingPlacement()){
			return ;
		}
	}

	//Rotation case
	//For the plane, it gives both rotations AND translations, depending on the state of the button
	if( (interactionMode == planeTouch || (interactionMode == dataPlaneTouch && settings->dataORplane == 1) || (interactionMode == seedPointTouch && settings->dataORplane == 1)) && fingerPositions.size() == 1){
		synchronized(fingerPositions){
			currentPos = Vector2(fingerPositions[0].x,fingerPositions[0].y);
		}
		synchronized(prevFingerPositions){
			prevPos = Vector2(prevFingerPositions[0].x, prevFingerPositions[0].y);
		}

		Vector2 diff = currentPos - prevPos ;
		diff *= settings->precision ;

		//LOGD("Diff = %f -- %f", diff.x, diff.y);

		//That's where we have to distinguish between translations and rotations
		//Depending on the state of translationPlane in settings

		//Translation case
		//LOGD("translatePlane value = %d",settings->translatePlane);
		if(settings->translatePlane){
			diff/= 10 ;
			//LOGD("Translate Plane %f -- %f", diff.x, diff.y);
			diff *= settings->precision ;
			diff *= settings->considerTranslation * settings->considerX * settings->considerY;
			Vector3 trans = Vector3(diff.x+diff.y, diff.x+diff.y, diff.x+diff.y);
			trans *= sliceNormal ;
			currentSlicePos +=trans ;
		}

		else{
			diff /=1000 ;
			Quaternion rot = currentSliceRot;
			rot = rot * Quaternion(rot.inverse() * Vector3::unitZ(), 0);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitY(), -diff.x);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitX(), diff.y);
			//currentSliceRot = rot ; //Version with the plane moving freely in the world
			currentSliceRot = rot ;	
		}
		
	

		return ;
	}

	if(fingerPositions.size() == 1){
		//LOGD("Normal Finger Interaction 1 finger");
		synchronized(fingerPositions){
			currentPos = Vector2(fingerPositions[0].x,fingerPositions[0].y);
		}
		synchronized(prevFingerPositions){
			prevPos = Vector2(prevFingerPositions[0].x, prevFingerPositions[0].y);
		}

		Vector2 diff = currentPos - prevPos ;
		diff /=1000 ;
		diff *= settings->precision ;

		diff.x *= settings->considerY * settings->considerRotation ;
		diff.y *= settings->considerX * settings->considerRotation ;

		if(interactionMode == dataTouch || interactionMode == dataPlaneHybrid || interactionMode == dataHybrid || (interactionMode == dataPlaneTouch && settings->dataORplane == 0) || interactionMode == seedPointHybrid || (interactionMode == seedPointTouch && settings->dataORplane==0)){
			//LOGD("Data interaction");
			Quaternion rot = currentDataRot;
			rot = rot * Quaternion(rot.inverse() * Vector3::unitZ(), 0);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitY(), -diff.x);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitX(), diff.y);
			currentDataRot = rot;
		}
	}

	else if(fingerPositions.size() == 2){
		//LOGD("Two finger interaction");
		Vector2 diff = Vector2(0,0);
		//Scale Factor update done Java Side
		//Nothing to do

		//Translation Computation
		for(int i = 0 ; i < 2 ; i++){
			synchronized(fingerPositions){
			currentPos = Vector2(fingerPositions[i].x,fingerPositions[i].y);
			}
			synchronized(prevFingerPositions){
				prevPos = Vector2(prevFingerPositions[i].x, prevFingerPositions[i].y);
			}

			diff += currentPos - prevPos ;
		}

		//FIXME Hardcoded
		diff /=2 ;
		diff /=4 ;
		diff *= settings->precision ;
		diff *= settings->considerTranslation * settings->considerX * settings->considerY;

		Vector3 trans = Vector3(diff.x, diff.y, 0);
		//LOGD("Diff = %f -- %f", diff.x, diff.y);
		

		if(interactionMode == planeTouch){
			currentSlicePos +=trans ;
		}
		else if(interactionMode == dataTouch || interactionMode == dataPlaneHybrid || (interactionMode == dataPlaneTouch && settings->dataORplane == 0) || (interactionMode == seedPointTouch && settings->dataORplane == 0) || interactionMode == seedPointHybrid ){
			currentDataPos +=trans ;	
		}


		//Rotation on the z axis --- Spinning 
		float x1,x2,y1,y2 ;
		synchronized(fingerPositions){
			x1 = fingerPositions[0].x ;
			x2 = fingerPositions[1].x ;
			y1 = fingerPositions[0].y ;
			y2 = fingerPositions[1].y ;
		}
        
        Vector2 newVec = Vector2(x2-x1,y2-y1);

        float dot = initialVector.x * newVec.x + initialVector.y * newVec.y ;
        float det = initialVector.x * newVec.y - initialVector.y * newVec.x ;

        float angle = atan2(det,dot);
        //FIXME : to correct for the case when I remove one finger and put it back, rotate 360°
        if(angle == 3.141593){	//FIXME hardcoded
        	angle = 0 ;	
        }

        angle *=settings->precision ;
        angle *= settings->considerZ * settings->considerRotation ;

        if(interactionMode == planeTouch){
			Quaternion rot = currentSliceRot;
			rot = rot * Quaternion(rot.inverse() * Vector3::unitZ(), angle);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitY(), 0);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitX(), 0);
			//currentSliceRot = rot ; //Version with the plane moving freely in the world
			currentSliceRot = rot ;
		}
		else if(interactionMode == dataTouch || interactionMode == dataPlaneHybrid || interactionMode == dataHybrid || (interactionMode == dataPlaneTouch && settings->dataORplane == 0) || interactionMode == seedPointHybrid || (interactionMode == seedPointTouch && settings->dataORplane == 0) ){
			Quaternion rot = currentDataRot;
			rot = rot * Quaternion(rot.inverse() * Vector3::unitZ(), angle);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitY(), 0);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitX(), 0);
			currentDataRot = rot;
		}

		//LOGD("Angle == %f", angle);
		//LOGD("New Vector = %f -- %f", newVec.x, newVec.y);
		//LOGD("Initial Vector = %f -- %f", initialVector.x, initialVector.y);

		//We set the initialVector to the new one, because relative mode
		initialVector = newVec ;
		

	}
	
}

/*void FluidMechanics::Impl::computeFingerInteraction(){
	Vector2 currentPos ;
	Vector2 prevPos ;

	//Particle seeding case
	//LOGD("ComputeFingerInteraction Function");
	//LOGD("%d == %d   ---  %d", interactionMode, seedPoint, fingerPositions.size());
	if(interactionMode == seedPoint && fingerPositions.size() == 1){
		
		if(computeSeedingPlacement()){
			return ;
		}
	}

	//Rotation case
	//For the plane, it gives both rotations AND translations, depending on the state of the button
	if(fingerPositions.size() == 1){
		synchronized(fingerPositions){
			currentPos = fingerPositions[0];
		}
		synchronized(prevFingerPositions){
			prevPos = prevFingerPositions[0];
		}

		Vector2 diff = currentPos - prevPos ;
		diff /=1000 ;
		diff *= settings->precision ;

		diff.x *= settings->considerY * settings->considerRotation ;
		diff.y *= settings->considerX * settings->considerRotation ;

		//LOGD("Diff = %f -- %f", diff.x, diff.y);

		if(interactionMode == planeTouch){
			//That's where we have to distinguish between translations and rotations
			//Depending on the state of translationPlane in settings

			//Translation case
			LOGD("translatePlane value = %d",settings->translatePlane);
			if(settings->translatePlane){
				LOGD("Translate Plane %f -- %f", diff.x, diff.y);
				diff *= settings->precision ;
				diff *= settings->considerTranslation * settings->considerX * settings->considerY;
				Vector3 trans = Vector3(diff.x, diff.y, 0);
				currentSlicePos +=trans ;
			}

			else{
				Quaternion rot = currentSliceRot;
				rot = rot * Quaternion(rot.inverse() * Vector3::unitZ(), 0);
				rot = rot * Quaternion(rot.inverse() * Vector3::unitY(), -diff.x);
				rot = rot * Quaternion(rot.inverse() * Vector3::unitX(), diff.y);
				//currentSliceRot = rot ; //Version with the plane moving freely in the world
				currentSliceRot = rot ;	
			}
			
		}
		else if(interactionMode == dataTouch || interactionMode == dataPlaneHybrid || interactionMode == dataHybrid){
			Quaternion rot = currentDataRot;
			rot = rot * Quaternion(rot.inverse() * Vector3::unitZ(), 0);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitY(), -diff.x);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitX(), diff.y);
			currentDataRot = rot;
		}
	}

	else if(fingerPositions.size() == 2){
		Vector2 diff = Vector2(0,0);
		//Scale Factor update done Java Side
		//Nothing to do

		//Translation Computation
		for(int i = 0 ; i < 2 ; i++){
			synchronized(fingerPositions){
			currentPos = fingerPositions[i];
			}
			synchronized(prevFingerPositions){
				prevPos = prevFingerPositions[i];
			}

			diff += currentPos - prevPos ;
		}

		//FIXME Hardcoded
		diff /=2 ;
		diff /=4 ;
		diff *= settings->precision ;
		diff *= settings->considerTranslation * settings->considerX * settings->considerY;

		Vector3 trans = Vector3(diff.x, diff.y, 0);
		//LOGD("Diff = %f -- %f", diff.x, diff.y);
		

		if(interactionMode == planeTouch){
			currentSlicePos +=trans ;
		}
		else if(interactionMode == dataTouch || interactionMode == dataPlaneHybrid){
			currentDataPos +=trans ;	
		}


		//Rotation on the z axis --- Spinning 
		float x1,x2,y1,y2 ;
		synchronized(fingerPositions){
			x1 = fingerPositions[0].x ;
			x2 = fingerPositions[1].x ;
			y1 = fingerPositions[0].y ;
			y2 = fingerPositions[1].y ;
		}
        
        Vector2 newVec = Vector2(x2-x1,y2-y1);

        float dot = initialVector.x * newVec.x + initialVector.y * newVec.y ;
        float det = initialVector.x * newVec.y - initialVector.y * newVec.x ;

        float angle = atan2(det,dot);
        //FIXME : to correct for the case when I remove one finger and put it back, rotate 360°
        if(angle == 3.141593){	//FIXME hardcoded
        	angle = 0 ;	
        }

        angle *=settings->precision ;
        angle *= settings->considerZ * settings->considerRotation ;

        if(interactionMode == planeTouch){
			Quaternion rot = currentSliceRot;
			rot = rot * Quaternion(rot.inverse() * Vector3::unitZ(), angle);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitY(), 0);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitX(), 0);
			//currentSliceRot = rot ; //Version with the plane moving freely in the world
			currentSliceRot = rot ;
		}
		else if(interactionMode == dataTouch || interactionMode == dataPlaneHybrid){
			Quaternion rot = currentDataRot;
			rot = rot * Quaternion(rot.inverse() * Vector3::unitZ(), angle);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitY(), 0);
			rot = rot * Quaternion(rot.inverse() * Vector3::unitX(), 0);
			currentDataRot = rot;
		}

		//LOGD("Angle == %f", angle);
		//LOGD("New Vector = %f -- %f", newVec.x, newVec.y);
		//LOGD("Initial Vector = %f -- %f", initialVector.x, initialVector.y);

		//We set the initialVector to the new one, because relative mode
		initialVector = newVec ;
		

	}
	
}*/

void FluidMechanics::Impl::updateMatrices(){
	Matrix4 statem ;
	Matrix4 slicem ;
	
	//LOGD("Tango Pos = %s", Utility::toString(currentSlicePos).c_str());
	//LOGD("Tango Rot = %s", Utility::toString(currentSliceRot).c_str());
	//LOGD("Precision = %f",settings->precision);
	//LOGD("ConstrainX = %d ; ConstrainY = %d ; ConstrainZ = %d", settings->considerX, settings->considerY, settings->considerZ );

	//LOGD("UPDATE MATRICES");
	//We need to call computeFingerInteraction() if the interaction mode uses tactile
	if(	interactionMode == dataTouch ||
	   	interactionMode == dataHybrid ||
	   	interactionMode == planeTouch ||
	   	interactionMode == dataPlaneTouch ||
	   	interactionMode == dataPlaneHybrid || 
	   	interactionMode == seedPointTangible ||
	   	interactionMode == seedPointTouch ||
	   	interactionMode == seedPointHybrid){

			//LOGD("Interaction Needs touch");
			computeFingerInteraction();
	}
	slicem = Matrix4::makeTransform(currentSlicePos, currentSliceRot);	//Version with the plane moving freely
	statem = Matrix4::makeTransform(currentDataPos, currentDataRot);


	//First we update the slice
	//Plane moving freely
	synchronized(state->stylusModelMatrix) {
		state->stylusModelMatrix = slicem;
	}

	//Plane not moving on the tablet
	/*synchronized(state->modelMatrix) {
		state->modelMatrix = m ;
	}*/
	


	//Then the data
	synchronized(state->modelMatrix) {
		state->modelMatrix = statem ;
	}
	
	

	updateSlicePlanes();

#if 0
	Matrix4 statem ;
	Matrix4 slicem ;
	//synchronized(state->modelMatrix) {
		//LOGD("Tango Pos = %s", Utility::toString(currentSlicePos).c_str());
		//LOGD("Tango Rot = %s", Utility::toString(currentSliceRot).c_str());
		//LOGD("Precision = %f",settings->precision);
		LOGD("ConstrainX = %d ; ConstrainY = %d ; ConstrainZ = %d", settings->considerX, settings->considerY, settings->considerZ );
		if(interactionMode == sliceTangibleOnly){
			m = Matrix4::makeTransform(currentSlicePos, currentSliceRot);	//Version with the plane moving freely
			//m = Matrix4::makeTransform(currentSlicePos, currentSliceRot.inverse());	//Fixed Plane on tablet
		}
		else if(interactionMode == dataTangibleOnly){
			m = Matrix4::makeTransform(currentDataPos, currentDataRot);
		}	
		else if(interactionMode == dataTouchOnly){
			computeFingerInteraction();
			m = Matrix4::makeTransform(currentDataPos, currentDataRot);
		}
		else if(interactionMode == seedPoint){
			computeFingerInteraction();
			m = Matrix4::makeTransform(currentSlicePos, currentSliceRot.inverse());	//Fixed Plane
			//m = Matrix4::makeTransform(currentDataPos, currentDataRot);
		}

	//}
	if(interactionMode == sliceTangibleOnly){
		//Plane moving freely
		synchronized(state->stylusModelMatrix) {
			state->stylusModelMatrix = m;
		}

		//Plane not moving on the tablet
		/*synchronized(state->modelMatrix) {
			state->modelMatrix = m ;
		}*/
	}
	else if(interactionMode == dataTangibleOnly || interactionMode == dataTouchOnly){
		synchronized(state->modelMatrix) {
			state->modelMatrix = m ;
		}
	}
	

	updateSlicePlanes();

#endif
}

std::string FluidMechanics::Impl::getData(){
	
  	std::ostringstream oss;
  	Matrix4 m ;

  	//First we set the zoomFactor
  	oss << settings->zoomFactor << ";" ;
  	int tmp = (settings->showVolume) ? 1 : 0 ;
  	oss << tmp << ";" ;
  	tmp = (settings->showSurface) ? 1 : 0 ;
  	oss << tmp << ";" ;
  	tmp = (settings->showStylus) ? 1 : 0 ;
  	oss << tmp << ";" ;
  	tmp = (settings->showSlice) ? 1 : 0 ;
  	oss << tmp << ";" ;
  	tmp = (settings->showOutline) ? 1 : 0 ;
  	oss << tmp << ";" ;

  	synchronized(state->modelMatrix){
  		m = state->modelMatrix ;	
  	}  	
	oss << m.data_[0] << ";" 
		<< m.data_[1] << ";" 
		<< m.data_[2] << ";" 
		<< m.data_[3] << ";" 
		<< m.data_[4] << ";" 
		<< m.data_[5] << ";" 
		<< m.data_[6] << ";" 
		<< m.data_[7] << ";" 
		<< m.data_[8] << ";" 
		<< m.data_[9] << ";" 
		<< m.data_[10] << ";" 
		<< m.data_[11] << ";" 
		<< m.data_[12] << ";" 
		<< m.data_[13] << ";" 
		<< m.data_[14] << ";" 
		<< m.data_[15] << ";" ;

	synchronized(state->stylusModelMatrix){
  		m = state->stylusModelMatrix ;	
  	}

	oss << m.data_[0] << ";" 
		<< m.data_[1] << ";" 
		<< m.data_[2] << ";" 
		<< m.data_[3] << ";" 
		<< m.data_[4] << ";" 
		<< m.data_[5] << ";" 
		<< m.data_[6] << ";" 
		<< m.data_[7] << ";" 
		<< m.data_[8] << ";" 
		<< m.data_[9] << ";" 
		<< m.data_[10] << ";" 
		<< m.data_[11] << ";" 
		<< m.data_[12] << ";" 
		<< m.data_[13] << ";" 
		<< m.data_[14] << ";" 
		<< m.data_[15] << ";" ;

	oss << seedingPoint.x << ";"
		<< seedingPoint.y << ";"  
		<< seedingPoint.z << ";" ;

	std::string s = oss.str();

	return s ;
}

void FluidMechanics::Impl::addFinger(float x, float y, int fingerID){
	Vector3 pos(x,y, fingerID);
	synchronized(prevFingerPositions){
		prevFingerPositions.push_back(pos);
	}
	synchronized(fingerPositions){
		fingerPositions.push_back(pos);
	}
	if(fingerPositions.size() == 2){
		float x1,x2,y1,y2 ;
		synchronized(fingerPositions){
			x1 = fingerPositions[0].x ;
			x2 = fingerPositions[1].x ;
			y1 = fingerPositions[0].y ;
			y2 = fingerPositions[1].y ;
		}
		initialVector = Vector2(x2-x1, y2-y1);
	}
}
void FluidMechanics::Impl::removeFinger(int fingerID){
	int position = getFingerPos(fingerID);
	synchronized(prevFingerPositions){
		prevFingerPositions.erase(prevFingerPositions.begin()+position);
	}
	synchronized(fingerPositions){
		fingerPositions.erase(fingerPositions.begin()+position);
	}
}

int FluidMechanics::Impl::getFingerPos(int fingerID){
	for(int i = 0 ; i < fingerPositions.size() ; i++){
		if(fingerPositions[i].z == fingerID){
			return i ;
		}
	}
	return -1 ;
}


void FluidMechanics::Impl::updateFingerPositions(float x, float y, int fingerID){
	Vector3 pos(x,y, fingerID);
	/*if(fingerID >= fingerPositions.size()){
		LOGD("Error in Finger ID");
		return ;
	}*/
	
	int position = getFingerPos(fingerID);
	if(position == -1){
		//LOGD("Error in Finger ID");
		return ;
	}
	synchronized(prevFingerPositions){
		prevFingerPositions[position] = fingerPositions[position];	
	}
	synchronized(fingerPositions){
		fingerPositions[position] = pos ;	
	}
	//LOGD("Finger %d has moved from %f -- %f     to     %f -- %f",fingerID,prevFingerPositions[position].x,prevFingerPositions[position].y, x, y);
}


void FluidMechanics::Impl::updateSlicePlanes()
{
	if(!data){
		return ;
	}
	if (state->stylusVisible) {
		if (state->tangibleVisible) { // <-- because of posToDataCoords()
			// Effector 2
			const float size = 0.5f * (stylusEffectorDist + std::max(dataSpacing.x*dataDim[0], std::max(dataSpacing.y*dataDim[1], dataSpacing.z*dataDim[2])));
			Vector3 dataPos = posToDataCoords(state->stylusModelMatrix * Matrix4::makeTransform(Vector3(-size, 0, 0)*settings->zoomFactor) * Vector3::zero());

			if (dataPos.x >= 0 && dataPos.y >= 0 && dataPos.z >= 0
			    && dataPos.x < dataDim[0]*dataSpacing.x && dataPos.y < dataDim[1]*dataSpacing.y && dataPos.z < dataDim[2]*dataSpacing.z)
			{
				// const auto rayPlaneIntersection2 = [](const Vector3& rayPoint, const Vector3& rayDir, const Vector3& planePoint, const Vector3& planeNormal, float& t) -> bool {
				// 	float dot = rayDir.dot(planeNormal);
				// 	if (dot != 0) {
				// 		t = -(rayPoint.dot(planeNormal) - planeNormal.dot(planePoint)) / dot;
				// 		LOGD("rayPlaneIntersection2 %s %s %s %s => %f", Utility::toString(rayPoint).c_str(), Utility::toString(rayDir).c_str(), Utility::toString(planePoint).c_str(), Utility::toString(planeNormal).c_str(), t);
				// 		return true;
				// 	} else {
				// 		LOGD("rayPlaneIntersection2 %s %s %s %s => [dot=%f]", Utility::toString(rayPoint).c_str(), Utility::toString(rayDir).c_str(), Utility::toString(planePoint).c_str(), Utility::toString(planeNormal).c_str(), dot);
				// 		return false;
				// 	}
				// };
				const auto rayAABBIntersection = [](const Vector3& rayPoint, const Vector3& rayDir, const Vector3& aabbMin, const Vector3& aabbMax, float& tmin, float& tmax) -> bool {
					// http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-box-intersection/
					float tmin_ = (aabbMin.x - rayPoint.x) / rayDir.x;
					float tmax_ = (aabbMax.x - rayPoint.x) / rayDir.x;
					if (tmin_ > tmax_) std::swap(tmin_, tmax_);
					float tymin = (aabbMin.y - rayPoint.y) / rayDir.y;
					float tymax = (aabbMax.y - rayPoint.y) / rayDir.y;
					if (tymin > tymax) std::swap(tymin, tymax);
					if ((tmin_ > tymax) || (tymin > tmax_))
						return false;
					if (tymin > tmin_)
						tmin_ = tymin;
					if (tymax < tmax_)
						tmax_ = tymax;
					float tzmin = (aabbMin.z - rayPoint.z) / rayDir.z;
					float tzmax = (aabbMax.z - rayPoint.z) / rayDir.z;
					if (tzmin > tzmax) std::swap(tzmin, tzmax);
					if ((tmin_ > tzmax) || (tzmin > tmax_))
						return false;
					if (tzmin > tmin_)
						tmin_ = tzmin;
					if (tzmax < tmax_)
						tmax_ = tzmax;
					if ((tmin_ > tmax) || (tmax_ < tmin)) return false;
					if (tmin < tmin_) tmin = tmin_;
					if (tmax > tmax_) tmax = tmax_;
					// LOGD("tmin = %f, tmax = %f", tmin, tmax);
					return true;
				};

				// const auto getAABBNormalAt = [](Vector3 point, const Vector3& aabbMin, const Vector3& aabbMax) -> Vector3 {
				// 	const auto sign = [](float value) {
				// 		return (value >= 0 ? 1 : -1);
				// 	};

				// 	// http://www.gamedev.net/topic/551816-finding-the-aabb-surface-normal-from-an-intersection-point-on-aabb/#entry4549909
				// 	Vector3 normal = Vector3::zero();
				// 	float min = std::numeric_limits<float>::max();
				// 	float distance;

				// 	Vector3 extents = aabbMax-aabbMin;
				// 	Vector3 center = (aabbMax+aabbMin)/2;

				// 	point -= center;

				// 	LOGD("point = %s, extents = %s", Utility::toString(point).c_str(), Utility::toString(extents).c_str());

				// 	distance = std::abs(extents.x - std::abs(point.x));
				// 	if (distance < min) {
				// 		min = distance;
				// 		normal = sign(point.x) * Vector3::unitX();
				// 	}

				// 	distance = std::abs(extents.y - std::abs(point.y));
				// 	if (distance < min) {
				// 		min = distance;
				// 		normal = sign(point.y) * Vector3::unitY();
				// 	}

				// 	distance = std::abs(extents.z - std::abs(point.z));
				// 	if (distance < min) {
				// 		min = distance;
				// 		normal = sign(point.z) * Vector3::unitZ();
				// 	}

				// 	return normal;
				// };

				// Same as posToDataCoords(), but for directions (not positions)
				// (direction goes from the effector to the stylus: +X axis)
				Vector3 dataDir = state->modelMatrix.transpose().get3x3Matrix() * state->stylusModelMatrix.inverse().transpose().get3x3Matrix() * Vector3::unitX();

				// static const float min = 0.0f;
				// const float max = settings->zoomFactor;
				// float t;
				float tmin = 0, tmax = 10000;
				synchronized (effectorIntersection) {
					// effectorIntersection = Vector3::zero();
					effectorIntersectionValid = false;
					// if (rayPlaneIntersection2(dataPos, dataDir, Vector3(0, 0, 0), -Vector3::unitX(), t) && t >= min && t <= max)
					// 	effectorIntersection = dataCoordsToPos(dataPos + dataDir*t);
					// if (rayPlaneIntersection2(dataPos, dataDir, Vector3(dataDim[0]*dataSpacing.x, 0, 0), Vector3::unitX(), t) && t >= min && t <= max)
					// 	effectorIntersection = dataCoordsToPos(dataPos + dataDir*t);
					// if (rayPlaneIntersection2(dataPos, dataDir, Vector3(0, 0, 0), -Vector3::unitY(), t) && t >= min && t <= max)
					// 	effectorIntersection = dataCoordsToPos(dataPos + dataDir*t);
					// if (rayPlaneIntersection2(dataPos, dataDir, Vector3(0, dataDim[1]*dataSpacing.y, 0), Vector3::unitY(), t) && t >= min && t <= max)
					// 	effectorIntersection = dataCoordsToPos(dataPos + dataDir*t);
					// if (rayPlaneIntersection2(dataPos, dataDir, Vector3(0, 0, 0), -Vector3::unitZ(), t) && t >= min && t <= max)
					// 	effectorIntersection = dataCoordsToPos(dataPos + dataDir*t);
					// if (rayPlaneIntersection2(dataPos, dataDir, Vector3(0, 0, dataDim[2]*dataSpacing.z), Vector3::unitZ(), t) && t >= min && t <= max)
					// 	effectorIntersection = dataCoordsToPos(dataPos + dataDir*t);
					if (rayAABBIntersection(dataPos, dataDir, Vector3::zero(), Vector3(dataDim[0], dataDim[1], dataDim[2])*dataSpacing, tmin, tmax) && tmax > 0) {
						effectorIntersection = dataCoordsToPos(dataPos + dataDir*tmax);
						// effectorIntersectionNormal = state->modelMatrix.transpose().get3x3Matrix() * getAABBNormalAt(dataPos + dataDir*tmax, Vector3::zero(), Vector3(dataDim[0], dataDim[1], dataDim[2])*dataSpacing);
						// LOGD("intersection = %s", Utility::toString(posToDataCoords(effectorIntersection)).c_str());
						// LOGD("intersection normal = %s", Utility::toString(getAABBNormalAt(dataPos + dataDir*tmax, Vector3::zero(), Vector3(dataDim[0], dataDim[1], dataDim[2])*dataSpacing)).c_str());
						effectorIntersectionValid = true;
					}
				}


				if (buttonIsPressed) {
					// settings->showSurface = true;
					settings->surfacePreview = true;

					vtkNew<vtkPoints> points;
					points->InsertNextPoint(dataPos.x, dataPos.y, dataPos.z);
					vtkNew<vtkPolyData> polyData;
					polyData->SetPoints(points.GetPointer());
					probeFilter->SetInputData(polyData.GetPointer());
					probeFilter->Update();

					vtkDataArray* scalars = probeFilter->GetOutput()->GetPointData()->GetScalars();
					android_assert(scalars);
					unsigned int num = scalars->GetNumberOfTuples();
					android_assert(num > 0);
					double value = scalars->GetComponent(0, 0);
					static double prevValue = 0.0;
					if (prevValue != 0.0)
						value = lowPassFilter(value, prevValue, 0.5f);
					prevValue = value;
					double range[2] = { volume->getMinValue(), volume->getMaxValue() };
					// LOGD("probed value = %f (range = %f / %f)", value, range[0], range[1]);
					settings->surfacePercentage = (value - range[0]) / (range[1] - range[0]);

					// NOTE: updateSurfacePreview cannot be used to update isosurfaceLow
					// from settings->surfacePercentage, since isosurfaceLow and isosurface
					// have different value ranges, hence expect different percentages.
					// updateSurfacePreview();

					// Directly use setValue() instead
					synchronized_if(isosurfaceLow) {
						isosurfaceLow->setValue(value);
					}
				}
			} else {
				effectorIntersectionValid = false;

				// // settings->showSurface = false;
				// settings->showSurface = true;
				// settings->surfacePreview = true;
			}
		}
	}

	// if (!state->tangibleVisible) // && !state->stylusVisible)
	// 	app->resetThreshold(); // reset the threshold in case its value went too far due

	bool clipPlaneSet = false;

	if (settings->showSlice && slice) {
		switch (settings->sliceType) {
			case SLICE_CAMERA:
				clipPlaneSet = computeCameraClipPlane(slicePoint, sliceNormal);
				break;

			case SLICE_AXIS:
				clipPlaneSet = computeAxisClipPlane(slicePoint, sliceNormal);
				break;

			case SLICE_STYLUS:
				clipPlaneSet = computeStylusClipPlane(slicePoint, sliceNormal);
				break;
		}
	}

	if (clipPlaneSet) {
		synchronized_if(isosurface) { isosurface->setClipPlane(sliceNormal.x, sliceNormal.y, sliceNormal.z, -sliceNormal.dot(slicePoint)); }
		synchronized_if(isosurfaceLow) { isosurfaceLow->setClipPlane(sliceNormal.x, sliceNormal.y, sliceNormal.z, -sliceNormal.dot(slicePoint)); }
		synchronized_if(volume) { volume->setClipPlane(sliceNormal.x, sliceNormal.y, sliceNormal.z, -sliceNormal.dot(slicePoint)); }

		// pt: data space
		// dir: eye space
		const auto rayPlaneIntersection = [this](const Vector3& pt, const Vector3& dir, float& t) -> bool {
			// float dot = dir.dot(posToDataCoords(sliceNormal));
			// float dot = dataCoordsToPos(dir).dot(sliceNormal);
			float dot = dir.dot(sliceNormal);
			if (dot == 0)
				return false;
			// t = -(pt.dot(posToDataCoords(sliceNormal)) - sliceNormal.dot(slicePoint)) / dot;
			t = -(dataCoordsToPos(pt).dot(sliceNormal) - sliceNormal.dot(slicePoint)) / dot;
			// t = -(pt.dot(sliceNormal) - sliceNormal.dot(slicePoint)) / dot;
			// LOGD("t = %f", t);
			return true;
		};


		// Slice-cube intersection
		float t;
		Vector3 dir;
		synchronized(slicePoints) {
			slicePoints.clear();

			// dir = Vector3(dataDim[0]*dataSpacing.x, 0, 0);
			static const float min = 0.0f;
			// static const float max = 1.0f;
			// static const float max = 1.15f; // FIXME: why?
			// static const float max = settings->zoomFactor;
			// static const float max = 4.6f; // 10.5 (ironProt), 1.15 (head)
			// // D/NativeApp(24843): 2.135922 1.067961 103 1.000000
			// // D/NativeApp(24843): 3.235294 1.617647 68 1.000000
			// // D/NativeApp(24843): 1.074219 0.537109 64 3.200000
			const float max = settings->zoomFactor;// * settings->zoomFactor;

			// static const float max = 2*settings->zoomFactor*state->computedZoomFactor;
			// LOGD("%f %f %d %f", settings->zoomFactor, state->computedZoomFactor, dataDim[0], dataSpacing.x);

			// Same as dataCoordsToPos(), but for directions (not positions)
			dir = state->modelMatrix.inverse().transpose().get3x3Matrix() * (Vector3(dataDim[0]*dataSpacing.x, 0, 0));// / settings->zoomFactor);

			if (rayPlaneIntersection(Vector3(0, 0, 0), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(0, 0, 0)) + dir*t);
			if (rayPlaneIntersection(Vector3(0, dataDim[1]*dataSpacing.y, 0), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(0, dataDim[1]*dataSpacing.y, 0)) + dir*t);
			if (rayPlaneIntersection(Vector3(0, 0, dataDim[2]*dataSpacing.z), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(0, 0, dataDim[2]*dataSpacing.z)) + dir*t);
			if (rayPlaneIntersection(Vector3(0, dataDim[1]*dataSpacing.y, dataDim[2]*dataSpacing.z), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(0, dataDim[1]*dataSpacing.y, dataDim[2]*dataSpacing.z)) + dir*t);

			// dir = Vector3(0, dataDim[1]*dataSpacing.y, 0);
			dir = state->modelMatrix.inverse().transpose().get3x3Matrix() * (Vector3(0, dataDim[1]*dataSpacing.y, 0));// / settings->zoomFactor);
			if (rayPlaneIntersection(Vector3(0, 0, 0), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(0, 0, 0)) + dir*t);
			if (rayPlaneIntersection(Vector3(dataDim[0]*dataSpacing.x, 0, 0), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(dataDim[0]*dataSpacing.x, 0, 0)) + dir*t);
			if (rayPlaneIntersection(Vector3(0, 0, dataDim[2]*dataSpacing.z), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(0, 0, dataDim[2]*dataSpacing.z)) + dir*t);
			if (rayPlaneIntersection(Vector3(dataDim[0]*dataSpacing.x, 0, dataDim[2]*dataSpacing.z), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(dataDim[0]*dataSpacing.x, 0, dataDim[2]*dataSpacing.z)) + dir*t);

			// dir = Vector3(0, 0, dataDim[2]*dataSpacing.z);
			dir = state->modelMatrix.inverse().transpose().get3x3Matrix() * (Vector3(0, 0, dataDim[2]*dataSpacing.z));// / settings->zoomFactor);
			if (rayPlaneIntersection(Vector3(0, 0, 0), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(0, 0, 0)) + dir*t);
			if (rayPlaneIntersection(Vector3(dataDim[0]*dataSpacing.x, 0, 0), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(dataDim[0]*dataSpacing.x, 0, 0)) + dir*t);
			if (rayPlaneIntersection(Vector3(0, dataDim[1]*dataSpacing.y, 0), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(0, dataDim[1]*dataSpacing.y, 0)) + dir*t);
			if (rayPlaneIntersection(Vector3(dataDim[0]*dataSpacing.x, dataDim[1]*dataSpacing.y, 0), dir, t) && t >= min && t <= max)
				slicePoints.push_back(dataCoordsToPos(Vector3(dataDim[0]*dataSpacing.x, dataDim[1]*dataSpacing.y, 0)) + dir*t);

			// LOGD("slicePoints.size() = %d", slicePoints.size());
		}
	} else {
		synchronized_if(isosurface) { isosurface->clearClipPlane(); }
		synchronized_if(isosurfaceLow) { isosurfaceLow->clearClipPlane(); }
		synchronized_if(volume) { volume->clearClipPlane(); }
	}
}

// (GL context)
void FluidMechanics::Impl::renderObjects()
{
	updateMatrices();
	const Matrix4 proj = app->getProjMatrix();

	glEnable(GL_DEPTH_TEST);

	// // XXX: test
	// Matrix4 mm;
	// synchronized(state->modelMatrix) {
	// 	mm = state->modelMatrix;
	// }
	// glDisable(GL_BLEND);
	// synchronized_if(isosurface) {
	// 	glDepthMask(true);
	// 	glDisable(GL_CULL_FACE);
	// 	isosurface->render(proj, mm);
	// }
	// glEnable(GL_DEPTH_TEST);
	// synchronized_if(volume) {
	// 	// glDepthMask(false);
	// 	glDepthMask(true);
	// 	glEnable(GL_BLEND);
	// 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // modulate
	// 	// glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive
	// 	glDisable(GL_CULL_FACE);
	// 	volume->render(proj, mm);
	// }
	//
	// return; // XXX: test

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDepthMask(true); // requires "discard" in the shader where alpha == 0

	if (settings->clipDist > 0.0f) {
		// Set a depth value for the slicing plane
		Matrix4 trans = Matrix4::identity();
		// trans[3][2] = app->getDepthValue(settings->clipDist); // relative to trans[3][3], which is 1.0
		trans[3][2] = app->getDepthValue(sliceDepth);
		// LOGD("%s", Utility::toString(trans).c_str());

		trans[1][1] *= -1; // flip the texture vertically, because of "orthoProjMatrix"

		synchronized(slice) {
			slice->setOpaque(false);
			slice->render(app->getOrthoProjMatrix(), trans);
		}
	}

	// Stylus (paddle?) z-buffer occlusion
	// TODO: correct occlusion shape for the real *stylus*
	if (false && state->stylusVisible && cube /*&& (settings->sliceType != SLICE_STYLUS || slice->isEmpty())*/) {
		glColorMask(false, false, false, false);
		glDepthMask(true);

		// if (qcarVisible) {
		// 	static const Matrix4 transform = Matrix4::makeTransform(
		// 		Vector3::zero(),
		// 		Quaternion::identity(),
		// 		Vector3(63, 63, 3)/2
		// 	);
		//
		// 	synchronized (state->stylusModelMatrix) {
		// 		cube->render(proj, state->stylusModelMatrix*transform);
		// 	}
		// } else {
		// 	bool isRfduinoStylus = false;
		// 	try {
		// 		std::string curMarkerFileName = stylus.getMarkers().at(stylus.getCurrentMarkerID()).patternFileName;
		// 		isRfduinoStylus = (curMarkerFileName == "14.patt" || curMarkerFileName == "24.patt" || curMarkerFileName == "26.patt");
		// 	} catch (...) {
		// 		// ...
		// 	}

			synchronized (state->stylusModelMatrix) {
				// if (!isRfduinoStylus) {
					cube->render(proj, state->stylusModelMatrix
					             * Matrix4::makeTransform(
						             Vector3(10.0, 0, 10.0),
						             Quaternion::identity(),
						             Vector3(59, 40, 3)/2
					             ));
					cube->render(proj, state->stylusModelMatrix
					             * Matrix4::makeTransform(
						             Vector3(10.0, -10.0, -5.0),
						             // Quaternion(Vector3::unitX(),  2.146),
						             Quaternion(Vector3::unitX(),  2.09),
						             Vector3(59, 40, 3)/2
					             ));
					cube->render(proj, state->stylusModelMatrix
					             * Matrix4::makeTransform(
						             Vector3(10.0, 10.0, -5.0),
						             // Quaternion(Vector3::unitX(), -2.146),
						             Quaternion(Vector3::unitX(), -2.09),
						             Vector3(59, 40, 3)/2
					             ));
					// Handle
					if (cylinder) {
						cylinder->render(proj, state->stylusModelMatrix
						                 * Matrix4::makeTransform(
							                 Vector3(75.0, 0.0, 0.0),
							                 Quaternion(Vector3::unitY(), M_PI/2),
							                 Vector3(0.01f, 0.01f, 0.017f)*2
						                 ));
					}
				// } else {
				// 	cube->render(proj, state->stylusModelMatrix
				// 	             * Matrix4::makeTransform(
				// 		             Vector3(11.0, 0, 18.0),
				// 		             Quaternion::identity(),
				// 		             Vector3(57, 40, 3)/2
				// 	             ));
				// 	cube->render(proj, state->stylusModelMatrix
				// 	             * Matrix4::makeTransform(
				// 		             Vector3(11.0, 0, -18.0),
				// 		             Quaternion::identity(),
				// 		             Vector3(57, 40, 3)/2
				// 	             ));
				// 	cube->render(proj, state->stylusModelMatrix
				// 	             * Matrix4::makeTransform(
				// 		             Vector3(11.0, 18.0, 0.0),
				// 		             Quaternion(Vector3::unitX(), -M_PI/2),
				// 		             Vector3(57, 40, 3)/2
				// 	             ));
				// 	cube->render(proj, state->stylusModelMatrix
				// 	             * Matrix4::makeTransform(
				// 		             Vector3(11.0, -18.0, 0.0),
				// 		             Quaternion(Vector3::unitX(), M_PI/2),
				// 		             Vector3(57, 40, 3)/2
				// 	             ));
				// }
			}
		// }

		glColorMask(true, true, true, true);
	}

	if (settings->showStylus && state->stylusVisible && cube) {
		glDepthMask(true);
		glEnable(GL_CULL_FACE);

		Matrix4 smm;
		synchronized(state->stylusModelMatrix) {
			smm = state->stylusModelMatrix;
		}
#if 0
#ifndef NEW_STYLUS_RENDER
		// Effector
		static const Matrix4 transform1 = Matrix4::makeTransform(
			Vector3(0, 0, 0), // (after scaling)
			Quaternion::identity(),
			Vector3(10.0f)
		);
		cube->setColor(Vector3(1.0f));
		// LOGD("render 1");
		cube->render(proj, smm*transform1);
		// LOGD("render 1 success");
		// cube->render(qcarProjMatrix, mm*transform1);

		// Handle
		static const Matrix4 transform2 = Matrix4::makeTransform(
			// Vector3((130/*+105*/)*0.5f, 0, 0), // (after scaling)
			Vector3((130+25)*0.5f, 0, 0), // (after scaling)
			Quaternion::identity(),
			// Vector3((130/*+105*/)*0.5f, 5.0f, 5.0f)
			Vector3((130+25)*0.5f, 5.0f, 5.0f)
		);
		cube->setColor(Vector3(0.7f));
		// LOGD("render 2");
		cube->render(proj, smm*transform2);
		// LOGD("render 2 success");
		// cube->render(qcarProjMatrix, mm*transform2);

		if (state->tangibleVisible) { // <-- because of posToDataCoords()
			// Effector 2
			const float size = 0.5f * (stylusEffectorDist + std::max(dataSpacing.x*dataDim[0], std::max(dataSpacing.y*dataDim[1], dataSpacing.z*dataDim[2])));
			Vector3 dataPos = posToDataCoords(smm * Matrix4::makeTransform(Vector3(-size, 0, 0)*settings->zoomFactor) * Vector3::zero());
			if (dataPos.x >= 0 && dataPos.y >= 0 && dataPos.z >= 0
			    && dataPos.x < dataDim[0] && dataPos.y < dataDim[1] && dataPos.z < dataDim[2])
			{
				cube->setColor(Vector3(0.5f));
				// cube->setOpacity(1.0f);
			} else {
				cube->setColor(Vector3(1, 0.5, 0.5));
				// cube->setOpacity(0.5f);
			}

			const Matrix4 transform3 = Matrix4::makeTransform(
				Vector3(-size, 0, 0) * settings->zoomFactor,
				Quaternion::identity(),
				Vector3(2.0f * settings->zoomFactor)
				// Vector3(0.3f * settings->zoomFactor)
			);

			// LOGD("render 3");
			cube->render(proj, smm*transform3);
			// cube->render(qcarProjMatrix, mm*transform3);
			// particleSphere->render(proj, mm*transform3);
			// LOGD("render 3 success");
		}
#else
		if (settings->sliceType == SLICE_STYLUS) {
			// // Effector
			// static const Matrix4 transform1 = Matrix4::makeTransform(
			// 	Vector3(0, 0, 0), // (after scaling)
			// 	Quaternion::identity(),
			// 	Vector3(10.0f)
			// );
			// cube->setColor(Vector3(1.0f));
			// // LOGD("render 1");
			// cube->render(proj, smm*transform1);
			// // LOGD("render 1 success");
			// // cube->render(qcarProjMatrix, smm*transform1);
			//
			// // Handle
			// static const Matrix4 transform2 = Matrix4::makeTransform(
			// 	// Vector3((130/*+105*/)*0.5f, 0, 0), // (after scaling)
			// 	Vector3((130+25)*0.5f, 0, 0), // (after scaling)
			// 	Quaternion::identity(),
			// 	// Vector3((130/*+105*/)*0.5f, 5.0f, 5.0f)
			// 	Vector3((130+25)*0.5f, 5.0f, 5.0f)
			// );
			// cube->setColor(Vector3(0.7f));
			// // LOGD("render 2");
			// cube->render(proj, smm*transform2);
			// // LOGD("render 2 success");
			// // cube->render(qcarProjMatrix, smm*transform2);

		} else {
		const float size = 0.5f * (stylusEffectorDist + std::max(dataSpacing.x*dataDim[0], std::max(dataSpacing.y*dataDim[1], dataSpacing.z*dataDim[2])));

		// Handle
		const Matrix4 transform2 = Matrix4::makeTransform(
			// Vector3((130/*+105*/)*0.5f, 0, 0), // (after scaling)
			Vector3(-size*0.5*settings->zoomFactor, 0, 0), // (after scaling)
			Quaternion::identity(),
			// Vector3((130/*+105*/)*0.5f, 5.0f, 5.0f)
			Vector3(size*0.5*settings->zoomFactor, 2.0f, 2.0f)
		);

		if (!state->tangibleVisible) {
		// if (!state->tangibleVisible || settings->sliceType == SLICE_STYLUS) {
			// Handle
			cube->setColor(Vector3(0.7f));
			cube->render(proj, smm*transform2);

		// if (state->tangibleVisible) { // <-- because of posToDataCoords()
		} else { // <-- because of posToDataCoords()
			Vector3 effectorPos = smm * Matrix4::makeTransform(Vector3(-size, 0, 0)*settings->zoomFactor) * Vector3::zero();
			Vector3 dataPos = posToDataCoords(effectorPos);
			const bool insideVolume = (dataPos.x >= 0 && dataPos.y >= 0 && dataPos.z >= 0
				&& dataPos.x < dataDim[0]*dataSpacing.x && dataPos.y < dataDim[1]*dataSpacing.y && dataPos.z < dataDim[2]*dataSpacing.z);
			if (insideVolume) {
				// cube->setColor(Vector3(0.5f));
				// cube->setColor(!mousePressed ? Vector3(0.5f) : Vector3(0.5f, 1.0f, 0.5f));
				cube->setColor(Vector3(0.5f));
				// cube->setOpacity(1.0f);
			} else {
				cube->setColor(Vector3(1, 0.5, 0.5));
				// cube->setOpacity(0.5f);
			}

			// Handle
			cube->render(proj, smm*transform2);

			// Effector 2
			const Matrix4 transform3 = Matrix4::makeTransform(
				Vector3(-size, 0, 0) * settings->zoomFactor,
				Quaternion::identity(),
				// Vector3(2.0f * settings->zoomFactor)
				Vector3(2.5f * settings->zoomFactor)
				// Vector3(0.3f * settings->zoomFactor)
			);

			// LOGD("render 3");
			cube->render(proj, smm*transform3);
			// cube->render(qcarProjMatrix, smm*transform3);
			// particleSphere->render(proj, smm*transform3);
			// LOGD("render 3 success");

			cube->setColor(Vector3(0.5f));

			if (insideVolume && settings->showCrossingLines) {
				// Show crossing axes to help the user locate
				// the effector position in the data
				Matrix4 mm;
				synchronized(state->modelMatrix) {
					mm = state->modelMatrix;
				}

				glLineWidth(2.0f);
				axisCube->setColor(Vector3(1.0f));
				// axisCube->setColor(mousePressed ? Vector3(1.0f) : Vector3(0.7f));

				axisCube->render(proj, mm*Matrix4::makeTransform(mm.inverse()*effectorPos*Vector3(0,1,1), Quaternion::identity(), Vector3(0.5*dataDim[0]*dataSpacing.x*settings->zoomFactor, 0, 0)));
				cube->render(proj, mm*Matrix4::makeTransform(mm.inverse()*effectorPos*Vector3(0,1,1)-Vector3(0.5*dataDim[0]*dataSpacing.x*settings->zoomFactor,0,0), Quaternion::identity(), Vector3(0.25, 2, 2)));
				cube->render(proj, mm*Matrix4::makeTransform(mm.inverse()*effectorPos*Vector3(0,1,1)+Vector3(0.5*dataDim[0]*dataSpacing.x*settings->zoomFactor,0,0), Quaternion::identity(), Vector3(0.25, 2, 2)));

				axisCube->render(proj, mm*Matrix4::makeTransform(mm.inverse()*effectorPos*Vector3(1,0,1), Quaternion::identity(), Vector3(0, 0.5*dataDim[1]*dataSpacing.y*settings->zoomFactor, 0)));
				cube->render(proj, mm*Matrix4::makeTransform(mm.inverse()*effectorPos*Vector3(1,0,1)-Vector3(0,0.5*dataDim[1]*dataSpacing.y*settings->zoomFactor,0), Quaternion::identity(), Vector3(2, 0.25, 2)));
				cube->render(proj, mm*Matrix4::makeTransform(mm.inverse()*effectorPos*Vector3(1,0,1)+Vector3(0,0.5*dataDim[1]*dataSpacing.y*settings->zoomFactor,0), Quaternion::identity(), Vector3(2, 0.25, 2)));

				axisCube->render(proj, mm*Matrix4::makeTransform(mm.inverse()*effectorPos*Vector3(1,1,0), Quaternion::identity(), Vector3(0, 0, 0.5*dataDim[2]*dataSpacing.z*settings->zoomFactor)));
				cube->render(proj, mm*Matrix4::makeTransform(mm.inverse()*effectorPos*Vector3(1,1,0)-Vector3(0,0,0.5*dataDim[2]*dataSpacing.y*settings->zoomFactor), Quaternion::identity(), Vector3(2, 2, 0.25)));
				cube->render(proj, mm*Matrix4::makeTransform(mm.inverse()*effectorPos*Vector3(1,1,0)+Vector3(0,0,0.5*dataDim[2]*dataSpacing.y*settings->zoomFactor), Quaternion::identity(), Vector3(2, 2, 0.25)));
			}

#if 0
			synchronized (effectorIntersection) {
				if (effectorIntersectionValid) {
					// Effector intersection
					const Matrix4 transform4 = Matrix4::makeTransform(
						effectorIntersection,
						Quaternion::identity(),
						// Vector3::unitZ().rotationTo(effectorIntersectionNormal),
						Vector3(3.0f * settings->zoomFactor)
					);
					// cube->render(proj, smm*transform4);
					cube->render(proj, transform4);
				}
			}
#endif
		}
		}
#endif
#endif
	}

	// LOGD("render 4");

	if (state->tangibleVisible) {
		Matrix4 mm;
		synchronized(state->modelMatrix) {
			mm = state->modelMatrix;
		}

		// Apply the zoom factor
		mm = mm * Matrix4::makeTransform(
			Vector3::zero(),
			Quaternion::identity(),
			Vector3(settings->zoomFactor)
		);

		//Render the outline
		if(settings->showOutline){
			synchronized_if(outline) {
				glDepthMask(true);
				glLineWidth(2.0f);
				outline->setColor(!tangoEnabled ? Vector3(1.0f, 0, 0) : Vector3(0, 1.0f, 0));
				//outline->setColor(!velocityData ? Vector3(1.0f, 0, 0) : Vector3(0, 1.0f, 0));
				outline->render(proj, mm);
			}
		}

		// Render the surface
		if (settings->showSurface) {
			glDisable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(true);

			if (!settings->surfacePreview || !isosurfaceLow) {
				synchronized_if(isosurface) {
					isosurface->render(proj, mm);
				}
			}

			if (settings->surfacePreview) {
				synchronized_if(isosurfaceLow) {
					isosurfaceLow->render(proj, mm);
				}
			}
		}

		// Render particles
		synchronized (particles) {
			for (Particle& p : particles) {
				if (!p.valid)
					continue;
				integrateParticleMotion(p);
				if (!p.valid || p.delayMs > 0)
					continue;
				Vector3 pos = p.pos;
				pos -= Vector3(dataDim[0]/2, dataDim[1]/2, dataDim[2]/2) * dataSpacing;
				// particleSphere->render(proj, mm * Matrix4::makeTransform(pos, Quaternion::identity(), Vector3(0.3f)));
				// particleSphere->render(proj, mm * Matrix4::makeTransform(pos, Quaternion::identity(), Vector3(0.2f)));
				particleSphere->render(proj, mm * Matrix4::makeTransform(pos, Quaternion::identity(), Vector3(0.15f)));
			}
		}

		// NOTE: must be rendered before "slice" (because of
		// transparency sorting)
		if (settings->showSlice && state->clipAxis != CLIP_NONE && state->lockedClipAxis == CLIP_NONE) {
			Vector3 scale;
			Vector3 color;
			switch (state->clipAxis) {
				case CLIP_AXIS_X: case CLIP_NEG_AXIS_X: scale = Vector3(150, 0, 0); color = Vector3(1, 0, 0); break;
				case CLIP_AXIS_Y: case CLIP_NEG_AXIS_Y: scale = Vector3(0, 150, 0); color = Vector3(0, 1, 0); break;
				case CLIP_AXIS_Z: case CLIP_NEG_AXIS_Z: scale = Vector3(0, 0, 150); color = Vector3(0, 0, 1); break;
				case CLIP_NONE: android_assert(false);
			}

			const Matrix4 trans = Matrix4::makeTransform(
				Vector3::zero(),
				Quaternion::identity(),
				scale
			);

			glDepthMask(true);
			glLineWidth(5.0f);
			axisCube->setColor(color);
			axisCube->render(proj, mm*trans);
		}

		// FIXME: slight misalignment error?
		// const float sliceDot = (settings->showSlice ? sliceNormal.dot(Vector3::unitZ()) : 0);

		// Render the volume
		if (settings->showVolume) {// && sliceDot <= 0) {
			glEnable(GL_DEPTH_TEST);
			synchronized_if(volume) {
				// glDepthMask(false);
				glDepthMask(true);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // modulate
				// glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive
				glDisable(GL_CULL_FACE);
				volume->render(proj, mm);
			}
		}

		if (slice && settings->showSlice) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_CULL_FACE);
			glDepthMask(true); // requires "discard" in the shader where alpha == 0

			switch (settings->sliceType) {
				case SLICE_CAMERA: {
					if (settings->clipDist > 0.0f) {
						// Set a depth value for the slicing plane
						Matrix4 trans = Matrix4::identity();
						// trans[3][2] = app->getDepthValue(settings->clipDist); // relative to trans[3][3], which is 1.0
						trans[3][2] = app->getDepthValue(sliceDepth);
						// LOGD("%s", Utility::toString(trans).c_str());

						trans[1][1] *= -1; // flip the texture vertically, because of "orthoProjMatrix"

						synchronized(slice) {
							slice->setOpaque(false);
							slice->render(app->getOrthoProjMatrix(), trans);
						}
					}

					break;
				}

				case SLICE_AXIS:
				case SLICE_STYLUS: {
					if (settings->sliceType != SLICE_STYLUS || state->stylusVisible) {
						Matrix4 s2mm;
						synchronized(state->sliceModelMatrix) {
							s2mm = state->sliceModelMatrix;
						}

						synchronized(slice) {
							slice->setOpaque(true || slice->isEmpty() /*settings->sliceType == SLICE_STYLUS || slice->isEmpty()*/);
							slice->render(proj, s2mm);
						}

#if 0
						synchronized(slicePoints) {
							// for (const Vector3& pt : slicePoints) {
							// 	// LOGD("pt = %s", Utility::toString(pt).c_str());
							// 	const Matrix4 transform1 = Matrix4::makeTransform(
							// 		// Vector3(0, 0, 0), // (after scaling)
							// 		pt,
							// 		Quaternion::identity(),
							// 		Vector3(10.0f)
							// 	);
							// 	cube->setColor(Vector3(1.0f));
							// 	cube->setOpacity(1.0f);
							// 	// cube->render(proj, Matrix4::makeTransform(pt)*transform1);
							// 	cube->render(proj, transform1);
							// }
							// // LOGD("============");
							if (!slicePoints.empty()) {
								// Vector3 center = Vector3::zero();
								// for (const Vector3& pt : slicePoints) {
								// 	center += pt;
								// }
								// center /= slicePoints.size();

								// std::vector<Vector3> lineVec;
								// std::map<unsigned int, std::map<unsigned int, float>> graph;
								// for (unsigned int i = 0; i < slicePoints.size(); ++i) {
								// 	for (unsigned int j = 0; j < slicePoints.size(); ++j) {
								// 		if (i == j) // || (graph.count(j) && graph.at(j).count(i)))
								// 			continue;
								// 		const Vector3 pt1 = slicePoints.at(i);
								// 		const Vector3 pt2 = slicePoints.at(j);
								// 		graph[i][j] = (pt2 - pt1).normalized().dot((center - pt1).normalized());
								// 	}
								// }

								// for (const auto& pair : graph) {
								// 	typedef std::pair<unsigned int, float> PairT;
								// 	std::vector<PairT> dots;
								// 	for (const auto& pair2 : pair.second) {
								// 		dots.push_back(PairT(pair2.first, pair2.second));
								// 	}
								// 	// Get the two edges with the lowest dot products relative to "center"
								// 	std::sort(dots.begin(), dots.end(), [](const PairT& a, const PairT& b) { return a.second < b.second; });
								// 	dots.resize(2);
								// 	for (const auto& pair3 : dots) {
								// 		lineVec.push_back(slicePoints.at(pair.first));
								// 		lineVec.push_back(slicePoints.at(pair3.first));
								// 	}
								// }



								std::vector<Vector3> lineVec;
								// std::set<std::pair<unsigned int, unsigned int>> pairs;
								// struct LineStruct { Vector3 p1, p2; float dist; };
								// std::map<std::pair<unsigned int, unsigned int>, LineStruct> pairs;
								std::map<unsigned int, std::map<unsigned int, float>> graph;
								for (unsigned int i = 0; i < slicePoints.size(); ++i) {
									for (unsigned int j = 0; j < slicePoints.size(); ++j) {
										// std::pair<unsigned int, unsigned int> pair(i, j);
										// if (i == j || pairs.count(pair))
										if (i == j || (graph.count(j) && graph.at(j).count(i)))
											continue;
										const Vector3 pt1 = slicePoints.at(i);
										const Vector3 pt2 = slicePoints.at(j);
										const Vector3 dpt1 = posToDataCoords(pt1);
										const Vector3 dpt2 = posToDataCoords(pt2);
										static const float epsilon = 0.1f;
										if (std::abs(dpt1.x-dpt2.x) < epsilon || std::abs(dpt1.y-dpt2.y) < epsilon || std::abs(dpt1.z-dpt2.z) < epsilon) {
										// float dot = (pt2 - pt1).normalized().dot((center - pt1).normalized());
										// LOGD("dot = %f", dot);
										// if (dot < 0.9f) {
											lineVec.push_back(pt1);
											lineVec.push_back(pt2);
										}
											// pairs.insert(pair);
											// graph[i][j] = pt1.distance(pt2);
										// }
									}
								}
								// // for (auto& pair : graph) {
								// // 	// if (pair.second.size() <= 2) {
								// // 	// 	for (auto& pair3 : pair.second) {
								// // 	// 		lineVec.push_back(slicePoints.at(pair.first));
								// // 	// 		lineVec.push_back(slicePoints.at(pair3.first));
								// // 	// 	}
								// // 	// 	continue;
								// // 	// }
								// // 	typedef std::pair<unsigned int, float> PairT;
								// // 	std::vector<PairT> dists;
								// // 	for (auto& pair2 : pair.second) {
								// // 		dists.push_back(PairT(pair2.first, pair2.second));
								// // 	}
								// // 	// std::sort(dists.begin(), dists.end(), std::lower<float>());
								// // 	std::sort(dists.begin(), dists.end(), [](const PairT& a, const PairT& b) { return a.second < b.second; });
								// // 	dists.resize(2);
								// // 	for (auto& pair3 : dists) {
								// // 		lineVec.push_back(slicePoints.at(pair.first));
								// // 		lineVec.push_back(slicePoints.at(pair3.first));
								// // 	}
								// // }
								lines->setLines(lineVec);
								// glLineWidth(1.0f);
								glLineWidth(2.0f);
								lines->setColor(Vector3(0, 1, 0));
								lines->render(proj, Matrix4::identity());
							}
						}
#endif
					}

					break;
				}
			}
		}

		// // Render the volume after the slicing plane when the plane
		// // normal is facing the screen
		// if (settings->showVolume && sliceDot > 0) {
		// 	synchronized_if(volume) {
		// 		glDepthMask(false);
		// 		glEnable(GL_BLEND);
		// 		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // modulate
		// 		// glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive
		// 		glDisable(GL_CULL_FACE);
		// 		volume->render(proj, mm);
		// 	}
		// }
	}

	// // XXX: debug
	// if (!settings->showVolume && !settings->showSurface) {
	// 	glEnable(GL_CULL_FACE);
	// 	synchronized(tangible) {
	// 		for (const auto& pair : tangible.getMarkers()) {
	// 			if (!pair.second.isVisible())
	// 				continue;
	//
	// 			// LOGD("marker %2d (main=%d) err=%f cf=%f", pair.first, pair.second.isMain, pair.second.err, pair.second.cf);
	//
	// 			// Center the cube on its native, and scale it up
	// 			const Matrix4 transform = Matrix4::makeTransform(
	// 				Vector3(0, 0, 0.5f), // (after scaling)
	// 				Quaternion::identity(),
	// 				Vector3(20.0f, 20.0f, 1.0f) * (pair.second.width/51.0)
	// 			);
	//
	// 			if (pair.first == tangible.getCurrentMarkerID())
	// 				cube->setColor(Vector3(0.25f, 1.0f, 0.25f));
	// 			else if (pair.second.dubious)
	// 				cube->setColor(Vector3(1.0f, 0.25f, 0.25f));
	// 			else
	// 				cube->setColor(Vector3(0.25f, 0.25f, 1.0f));
	//
	// 			cube->render(proj, pair.second.transform*transform);
	//
	// 			// LOGD("modelMatrix for marker %d = %s", pair.first, Utility::toString(pair.second.transform).c_str());
	// 		}
	// 		// LOGD("==========");
	// 	}
	// }

	// if (qcarVisible) {
	// 	synchronized (qcarModelMatrix) {
	// 		const Matrix4 transform = Matrix4::makeTransform(
	// 			Vector3(0, 0, 0.5f), // (after scaling)
	// 			Quaternion::identity(),
	// 			Vector3(30.0f, 30.0f, 30.0f)
	// 		);

	// 		cube->render(proj, qcarModelMatrix*transform);
	// 	}
	// }
}


void FluidMechanics::Impl::updateSurfacePreview()
{
	if (!settings->surfacePreview) {
		synchronized_if(isosurface) {
			isosurface->setPercentage(settings->surfacePercentage);
		}
	} else if (settings->surfacePreview) {
		synchronized_if(isosurfaceLow) {
			isosurfaceLow->setPercentage(settings->surfacePercentage);
		}
	}
}

JNIEXPORT jboolean JNICALL Java_fr_limsi_ARViewer_FluidMechanics_loadDataset(JNIEnv* env,
	jobject obj, jstring filename)
{
	try {
		// LOGD("(JNI) [FluidMechanics] loadDataSet()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		const char* javaStr = env->GetStringUTFChars(filename, nullptr);
		if (!javaStr)
			throw std::runtime_error("GetStringUTFChars() returned null");
		const std::string filenameStr(javaStr);
		env->ReleaseStringUTFChars(filename, javaStr);

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		return instance->loadDataSet(filenameStr);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
		return false;
	}
}

JNIEXPORT jboolean JNICALL Java_fr_limsi_ARViewer_FluidMechanics_loadVelocityDataset(JNIEnv* env,
	jobject obj, jstring filename)
{
	try {
		// LOGD("(JNI) [FluidMechanics] loadVelocityDataSet()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		const char* javaStr = env->GetStringUTFChars(filename, nullptr);
		if (!javaStr)
			throw std::runtime_error("GetStringUTFChars() returned null");
		const std::string filenameStr(javaStr);
		env->ReleaseStringUTFChars(filename, javaStr);

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		return instance->loadVelocityDataSet(filenameStr);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
		return false;
	}
}

JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_releaseParticles(JNIEnv* env, jobject obj)
{
	try {
		// LOGD("(JNI) [FluidMechanics] releaseParticles()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		instance->releaseParticles();

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_getSettings(JNIEnv* env, jobject obj, jobject settingsObj)
{
	try {
		// LOGD("(JNI) [FluidMechanics] getSettings()");

		if (!settingsObj)
			throw std::runtime_error("\"Settings\" object is null");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		jclass cls = env->GetObjectClass(settingsObj);
		if (!cls)
			throw std::runtime_error("GetObjectClass() returned null");

		FluidMechanics::Settings* settings = dynamic_cast<FluidMechanics::Settings*>(App::getInstance()->getSettings().get());
		android_assert(settings);
		return settings->read(env, settingsObj, cls);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_setSettings(JNIEnv* env, jobject obj, jobject settingsObj)
{
	try {
		// LOGD("(JNI) [FluidMechanics] setSettings()");

		if (!settingsObj)
			throw std::runtime_error("\"Settings\" object is null");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		jclass cls = env->GetObjectClass(settingsObj);
		if (!cls)
			throw std::runtime_error("GetObjectClass() returned null");

		FluidMechanics::Settings* settings = dynamic_cast<FluidMechanics::Settings*>(App::getInstance()->getSettings().get());
		android_assert(settings);
		settings->write(env, settingsObj, cls);

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(settings);
		instance->updateSurfacePreview();

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_getState(JNIEnv* env, jobject obj, jobject stateObj)
{
	try {
		// LOGD("(JNI) [FluidMechanics] getState()");

		if (!stateObj)
			throw std::runtime_error("\"State\" object is null");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		jclass cls = env->GetObjectClass(stateObj);
		if (!cls)
			throw std::runtime_error("GetObjectClass() returned null");

		FluidMechanics::State* state = dynamic_cast<FluidMechanics::State*>(App::getInstance()->getState().get());
		android_assert(state);
		state->read(env, stateObj, cls);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_buttonPressed(JNIEnv* env, jobject obj)
{
	try {
		 //LOGD("(JNI) [FluidMechanics] buttonPressed()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		instance->buttonPressed();

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_setTangoValues(JNIEnv* env, jobject obj,jdouble tx, jdouble ty, jdouble tz, jdouble rx, jdouble ry, jdouble rz, jdouble q){
	try {
		// LOGD("(JNI) [FluidMechanics] releaseParticles()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		instance->setTangoValues(tx,ty,tz,rx,ry,rz,q);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void JNICALL Java_fr_limsi_ARViewer_FluidMechanics_setGyroValues(JNIEnv* env, jobject obj, jdouble rx, jdouble ry, jdouble rz, jdouble q){
	try {
		// LOGD("(JNI) [FluidMechanics] releaseParticles()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		instance->setGyroValues(rx,ry,rz,q);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT jfloat JNICALL Java_fr_limsi_ARViewer_FluidMechanics_buttonReleased(JNIEnv* env, jobject obj)
{
	try {
		 //LOGD("(JNI) [FluidMechanics] buttonReleased()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		return instance->buttonReleased();

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
		return 0.0f;
	}
}

JNIEXPORT jstring Java_fr_limsi_ARViewer_FluidMechanics_getData(JNIEnv* env, jobject obj)
{
	try {
		// LOGD("(JNI) [FluidMechanics] releaseParticles()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		return (env->NewStringUTF(instance->getData().c_str())) ;

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
	
}


JNIEXPORT void Java_fr_limsi_ARViewer_FluidMechanics_setInteractionMode(JNIEnv* env, jobject obj, jint mode){
	try {
		// LOGD("(JNI) [FluidMechanics] releaseParticles()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		instance->setInteractionMode(mode);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void Java_fr_limsi_ARViewer_FluidMechanics_updateFingerPositions(JNIEnv* env, jobject obj,jfloat x, jfloat y, jint fingerID){
	try {
		// LOGD("(JNI) [FluidMechanics] releaseParticles()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		instance->updateFingerPositions(x,y,fingerID);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void Java_fr_limsi_ARViewer_FluidMechanics_addFinger(JNIEnv* env, jobject obj,jfloat x, jfloat y, jint fingerID){
	try {
		// LOGD("(JNI) [FluidMechanics] releaseParticles()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		instance->addFinger(x,y,fingerID);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void Java_fr_limsi_ARViewer_FluidMechanics_reset(JNIEnv* env, jobject obj){
	try {
		// LOGD("(JNI) [FluidMechanics] releaseParticles()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		instance->reset();

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

JNIEXPORT void Java_fr_limsi_ARViewer_FluidMechanics_removeFinger(JNIEnv* env, jobject obj, jint fingerID){
	try {
		// LOGD("(JNI) [FluidMechanics] releaseParticles()");

		if (!App::getInstance())
			throw std::runtime_error("init() was not called");

		if (App::getType() != App::APP_TYPE_FLUID)
			throw std::runtime_error("Wrong application type");

		FluidMechanics* instance = dynamic_cast<FluidMechanics*>(App::getInstance());
		android_assert(instance);
		instance->removeFinger(fingerID);

	} catch (const std::exception& e) {
		throwJavaException(env, e.what());
	}
}

FluidMechanics::FluidMechanics(const InitParams& params)
 : NativeApp(params, SettingsPtr(new FluidMechanics::Settings), StatePtr(new FluidMechanics::State)),
   impl(new Impl(params.baseDir))
{
	impl->app = this;
	impl->settings = std::static_pointer_cast<FluidMechanics::Settings>(settings);
	impl->state = std::static_pointer_cast<FluidMechanics::State>(state);
}

bool FluidMechanics::loadDataSet(const std::string& fileName)
{
	impl->setMatrices(Matrix4::makeTransform(Vector3(0, 0, 400)),
	                  Matrix4::makeTransform(Vector3(0, 0, 400)));
	impl->currentSlicePos = Vector3(0, 0, 400);
	impl->currentDataPos = Vector3(0,0,400);
	return impl->loadDataSet(fileName);
}

bool FluidMechanics::loadVelocityDataSet(const std::string& fileName)
{
	return impl->loadVelocityDataSet(fileName);
}

void FluidMechanics::releaseParticles()
{
	impl->releaseParticles();
}

void FluidMechanics::buttonPressed()
{
	impl->buttonPressed();
}

float FluidMechanics::buttonReleased()
{
	return impl->buttonReleased();
}

void FluidMechanics::rebind()
{
	NativeApp::rebind();
	impl->rebind();
}

void FluidMechanics::setMatrices(const Matrix4& volumeMatrix, const Matrix4& stylusMatrix)
{
	impl->setMatrices(volumeMatrix, stylusMatrix);
}

void FluidMechanics::renderObjects()
{
	impl->renderObjects();
}

void FluidMechanics::updateSurfacePreview()
{
	impl->updateSurfacePreview();
}

void FluidMechanics::reset(){
	impl->reset();
}


void FluidMechanics::setInteractionMode(int mode){
	impl->setInteractionMode(mode);
}

std::string FluidMechanics::getData(){
	return impl->getData();
}

void FluidMechanics::updateFingerPositions(float x, float y, int fingerID){
	impl->updateFingerPositions(x,y,fingerID);
}

void FluidMechanics::addFinger(float x, float y, int fingerID){
	impl->addFinger(x,y,fingerID);
}

void FluidMechanics::removeFinger(int fingerID){
	impl->removeFinger(fingerID);
}

void FluidMechanics::setTangoValues(double tx, double ty, double tz, double rx, double ry, double rz, double q){
	impl->setTangoValues(tx,ty,tz,rx,ry,rz,q);
}

void FluidMechanics::setGyroValues(double rx, double ry, double rz, double q){
	impl->setGyroValues(rx,ry,rz,q);
}