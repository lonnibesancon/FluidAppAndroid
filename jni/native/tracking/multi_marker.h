#ifndef MULTI_MARKER_H
#define MULTI_MARKER_H

#include "global.h"

#include <list>
#include <map>
#include <set>

#include "AR/ar.h"

struct SubMarker
{
	SubMarker()
	 : err(-1), cf(-1),
	   blacklisted(false), dubious(false)
	{}

	SubMarker(const std::string& patternFileName_, double width_, const Matrix4& struct_, bool isMain_)
	 : patternFileName(patternFileName_), structure(struct_),
	   inverseStruct(struct_.inverse()),
	   width(width_), isMain(isMain_),
	   err(-1), cf(-1),
	   blacklisted(false), dubious(false)
	{}

	bool isVisible() const { return err > 0; }

	std::string patternFileName;
	Matrix4 structure, inverseStruct;
	double width;
	bool isMain;
	Matrix4 transform;
	double pattTrans[3][4];
	double err, cf;
	bool blacklisted, dubious;
};

class MultiMarker
{
public:
	enum VisibilityStatus { VISIBLE, INVISIBLE, TEMP_LOST };

	MultiMarker();
	MultiMarker(const std::list<SubMarker> subMarkers, const std::string& baseDir);

	~MultiMarker();

	const std::map<int, SubMarker>& getMarkers() const
	{ return mARIDToMarker; }

	int getCurrentMarkerID() const
	{ return mCurrentMarker; }

	VisibilityStatus getVisibility() const
	{ return mVisibility; }

	void setHysteresis(bool hysteresis);

	struct DetectionResult
	{
		bool visible;
		Matrix4 modelMatrix; // invalid when "visible == false"
	};

	DetectionResult detect(ARMarkerInfo* markerInfo, int markerNum);

	DetectionResult detectWithTolerance(
		ARMarkerInfo* markerInfo, int markerNum,
		const Matrix4& projMatrix, unsigned int delay // ms
	);

private:
	void freeMarkers();

	float distance(const Matrix4& m1, const Matrix4& m2);
	Matrix4 filter(const Matrix4& in, const Matrix4& prev);

	bool mInitialized;

	std::map<int, SubMarker> mARIDToMarker;
	bool mHysteresis;

	VisibilityStatus mVisibility;
	int mCurrentMarker;

	unsigned long mLastDetectionTime;
	Matrix4 mLastTransform;

	float mAvgDist;
	unsigned int mAvgSamples;
	static constexpr float avgWeight = 0.2f; // last 5 frames
};

#endif /* MULTI_MARKER_H */
