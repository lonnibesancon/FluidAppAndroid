#include "multi_marker.h"

#include <limits>

#include "AR/gsub.h"

MultiMarker::MultiMarker()
 : mInitialized(false),
   mVisibility(INVISIBLE)
{
	//
}

MultiMarker::MultiMarker(const std::list<SubMarker> subMarkers, const std::string& baseDir)
 : mInitialized(false),
   mHysteresis(true),
   mVisibility(INVISIBLE),
   mCurrentMarker(-1),
   mAvgDist(0), mAvgSamples(0)
{
	android_assert(!subMarkers.empty());

	for (const auto& marker : subMarkers) {
		// LOGD("MultiMarker: registering sub pattern \"%s/%s\"", baseDir.c_str(), marker.patternFileName.c_str());
		int id = arLoadPatt((baseDir + "/" + marker.patternFileName).c_str());
		if (id < 0) {
			freeMarkers();
			throw std::runtime_error("MultiMarker: error loading pattern");
		}
		mARIDToMarker[id] = marker;
		// LOGD("MultiMarker: %s => %d", marker.patternFileName.c_str(), id);
	}

	mInitialized = true;
}

MultiMarker::~MultiMarker()
{
	if (!mARIDToMarker.empty())
		freeMarkers();
}

void MultiMarker::freeMarkers()
{
	for (const auto& pair : mARIDToMarker) {
		const int id = pair.first;
		if (arFreePatt(id) >= 0)
			LOGD("MultiMarker: unloaded pattern %d", id);
		else
			LOGW("MultiMarker: unable to unload marker %d", id);
	}

	mARIDToMarker.clear();
}

void MultiMarker::setHysteresis(bool hysteresis)
{
	android_assert(mInitialized);
	mHysteresis = hysteresis;
}

MultiMarker::DetectionResult MultiMarker::detect(ARMarkerInfo* markerInfo, int markerNum)
{
	android_assert(mInitialized);
	android_assert(!mARIDToMarker.empty());

	DetectionResult result;

	std::set<int> invisibleMarkersIDs;
	for (const auto& pair : mARIDToMarker)
		invisibleMarkersIDs.insert(pair.first);

	double minErr = std::numeric_limits<double>::infinity();
	int bestMarker = -1;
	bool mainMarkerVisible = false;
	unsigned int numUsableMarkers = 0;

	// WARNING: the same marker ID may be reported twice in "markerInfo" !!
	for (int i = 0; i < markerNum; ++i) {
		const int id = markerInfo[i].id;

		if (id != -1 && mARIDToMarker.count(id)) {
			SubMarker& marker = mARIDToMarker.at(id);
			invisibleMarkersIDs.erase(id);

			if (!marker.blacklisted)
				++numUsableMarkers;

			marker.cf = markerInfo[i].cf;

			static double pattCenter[2] = {0.0, 0.0};
			if ((!marker.isMain && marker.dubious) || !marker.isVisible() || !mHysteresis)
				marker.err = arGetTransMat(&markerInfo[i], pattCenter, marker.width, marker.pattTrans);
			else
				marker.err = arGetTransMatCont(&markerInfo[i], marker.pattTrans, pattCenter, marker.width, marker.pattTrans);
			// android_assert(marker.err >= 0);

			double result[16];
			argConvGlpara(marker.pattTrans, result);

			for (int i = 0; i < 16; ++i)
				marker.transform.data_[i] = result[i];

			try {
				float zDot = (marker.transform.inverse().transpose().get3x3Matrix() * Vector3::unitZ()).normalized().dot(-Vector3::unitZ());
				// float prevErr = marker.err;
				// if (!hasMain)
				// 	marker.isMainTmp = (zDot < 0.85 && zDot > 0.5);
				if (marker.width <= 30.0 && marker.isMain) { // XXX: hack
					marker.err += 3.0 * std::exp(-std::pow(1-zDot, 2) / 0.01); // penalty for zDot > 0.85
					// marker.err += 3.0 * (zDot > 0.34 ? std::exp(-std::pow(3*(zDot-0.34), 2) / 0.1) : 1.0); // penalty for zDot < ~0.5
				}
				// LOGD("[%2d] zDot = %f, err = %f => %f", id, zDot, prevErr, marker.err);
				// LOGD("[%2d] area %d", id, markerInfo[i].area);
			} catch (...) {}
		}
	}

	// Reset the error value and cf value of all invisible markers to -1
	for (int id : invisibleMarkersIDs) {
		SubMarker& marker = mARIDToMarker.at(id);
		marker.err = -1;
		marker.cf = -1;
	}

	// Only blacklist markers if we have enough consecutive previous
	// frames to be able to tell wrong markers apart
	if (mAvgSamples >= 1/avgWeight) {
		for (auto& pair : mARIDToMarker) {
			SubMarker& marker = pair.second;
			if (!marker.isVisible())
				continue;
			Matrix4 mm = marker.transform * marker.inverseStruct;
			// Check if the computed position is "unacceptably"
			// different from the previous transform (ie. the
			// difference is more than 10 times the average difference
			// from the last 1/avgWeight frames)
			float dist = distance(mm, mLastTransform);
			if (dist > mAvgDist*10) { // FIXME: arbitrary and hardcoded value: *10
				float zDot = (marker.transform.inverse().transpose().get3x3Matrix() * Vector3::unitZ()).normalized().dot(-Vector3::unitZ());
				if (!marker.isMain || (marker.width <= 30.0 && zDot > 0.80 && numUsableMarkers > 1)) {
					--numUsableMarkers;
					// LOGD("blacklist marker %d (%f > %f)", id, dist, mAvgDist*10);
					marker.blacklisted = true;
					marker.dubious = true;
				}
			} else {
				// if (marker.blacklisted)
				// 	LOGD("un-blacklist marker %d", id);
				marker.blacklisted = false;
				marker.dubious = false;
			}
		}
	}

	// Find the "best" marker among the visible and non-blacklisted
	// markers
	for (const auto& pair : mARIDToMarker) {
		const int id = pair.first;
		const SubMarker& marker = pair.second;

		if (!marker.isVisible() || marker.blacklisted)
			continue;

		if (bestMarker == -1
		    || (marker.err < minErr && (!mainMarkerVisible || marker.isMain)))
		{
			minErr = marker.err;
			bestMarker = id;
			mainMarkerVisible = marker.isMain;
		}
	}

	if (bestMarker != -1) {
		// Try to stick to the same marker as long as possible (until
		// it disappears, or until its error value reaches a given
		// threshold). This helps reducing the "jumpiness" when switching
		// between markers, due to small imperfections in the physical prop.
		if (mCurrentMarker != -1) {
			const SubMarker& cm = mARIDToMarker.at(mCurrentMarker);
			if (cm.isVisible() && cm.err < 3.0 // FIXME: hardcoded error threshold
			    && !cm.blacklisted
			    && (!mainMarkerVisible || cm.isMain))
			{
				if (bestMarker != mCurrentMarker) {
					const SubMarker& cm = mARIDToMarker.at(mCurrentMarker);
					float zDot = (cm.transform.inverse().transpose().get3x3Matrix() * Vector3::unitZ()).normalized().dot(-Vector3::unitZ());
					if (cm.isVisible() && cm.err < 3.0 // FIXME: hardcoded error threshold
					    // if (cm.isVisible() && cm.err < 0.35 // FIXME: hardcoded error threshold
					    && !cm.blacklisted
					    // && (!mainMarkerVisible || cm.isMain))
					    && (!mainMarkerVisible || (cm.isMain && (cm.width > 30.0 || zDot < 0.80))))
					{
						// LOGD("dist = %f", distance(mARIDToMarker.at(bestMarker).transform, cm.transform));
						bestMarker = mCurrentMarker;
					}
				}
			}
		}

		const SubMarker& bm = mARIDToMarker.at(bestMarker);
		android_assert(bm.isVisible());
		result.modelMatrix = bm.transform * bm.inverseStruct;

		// As soon as we find a "main" marker, reset the blacklisted
		// state of all markers (to prevent secondary markers from
		// ending up being all blacklisted)
		if (bm.isMain) {
			for (auto& pair : mARIDToMarker)
				pair.second.blacklisted = false;
		}

		// If the object was already visible, filter the raw pose
		if (mCurrentMarker != -1) {
			float dist = distance(result.modelMatrix, mLastTransform);
			// LOGD("dist = %f", dist);

			++mAvgSamples;

			if (mAvgSamples >= 1) {
				mAvgDist = dist*avgWeight + mAvgDist*(1-avgWeight);

			} else if (mAvgSamples == 1) {
				mAvgDist = dist;
			}

			result.modelMatrix = filter(result.modelMatrix, mLastTransform);
		}

		mCurrentMarker = bestMarker;
		result.visible = true;
		mVisibility = VISIBLE;

		timespec now;
		clock_gettime(CLOCK_REALTIME, &now);
		mLastDetectionTime = (now.tv_sec*1000UL + now.tv_nsec / 1000000UL);
		mLastTransform = result.modelMatrix;

	} else {
		mCurrentMarker = -1;
		result.visible = false;
		mVisibility = INVISIBLE;

		mAvgSamples = 0;
	}

	return result;
}

extern int checkStylus; // XXX: test
MultiMarker::DetectionResult MultiMarker::detectWithTolerance(
	ARMarkerInfo* markerInfo, int markerNum,
	const Matrix4& projMatrix, unsigned int delay)
{
	DetectionResult result = detect(markerInfo, markerNum);

	const bool isStylus = mARIDToMarker.count(14); // XXX: test

	if (result.visible) {
		if (isStylus) checkStylus = true; // XXX: test
		return result;

	} else if (mLastDetectionTime != 0) {
		timespec now;
		clock_gettime(CLOCK_REALTIME, &now);
		unsigned long time = (now.tv_sec*1000UL + now.tv_nsec / 1000000UL);

		if (time - mLastDetectionTime < delay) {
			try {
				// Check if the tracked object was not close to the screen
				// the last time it was detected, to prevent "delays" when
				// the object goes outside the field of view.
				Vector3 screenPos = projMatrix * mLastTransform.position();
				// LOGD("screenPos = %f %f", screenPos.x, screenPos.y);
				if (!isStylus && std::abs(screenPos.x) < 0.4f && std::abs(screenPos.y) < 0.4f) {
					// The object was not near the screen borders, and the
					// tracking interruption was short enough: pretend the
					// object didn't move.
					result.visible = true;
					mVisibility = TEMP_LOST;
					result.modelMatrix = mLastTransform;
				}
				if (isStylus) checkStylus = true; // XXX: test
			} catch (...) {
				// ignore errors (mLastTransform may be invalid)
			}
		}
	}

	// XXX: test
	if (!result.visible && isStylus) {
		checkStylus = false;
	}

	return result;
}

// From: "Jittering Reduction in Marker-Based Augmented Reality Systems"
Matrix4 MultiMarker::filter(const Matrix4& in, const Matrix4& prev)
{
	// TODO: Kalman filter for position?

	// FIXME: hardcoded
	static const float angleWeight = 0.50f;
	static const float posWeight   = 0.75f;

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
				// Average the last 1/angleWeight values
				result[col][row] = in[col][row]*angleWeight + prev[col][row]*(1-angleWeight);

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

// Compute the sum of distances between each matrix value (position
// terms are given more weight)
float MultiMarker::distance(const Matrix4& m1, const Matrix4& m2)
{
	float result = 0;

	for (unsigned int col = 0; col < 4; ++col) {
		// Skip the last row
		for (unsigned int row = 0; row < 3; ++row) {
			float diff = std::abs(m1[col][row] - m2[col][row]);

			// Position differences are more noticeable, so they are
			// given more weight
			if (col == 3)
				diff *= 4;

			result += diff;
		}
	}

	return result;
}
