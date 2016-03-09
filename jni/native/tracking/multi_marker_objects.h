#ifndef MULTI_MARKER_OBJECTS_H
#define MULTI_MARKER_OBJECTS_H

#include "global.h"

#include "multi_marker.h"

namespace MultiMarkerObjects {
	const std::list<SubMarker> tangible = {
		{   "4.patt", 47.0, Matrix4::makeTransform(Vector3(      0,        0,  76.0/2), Quaternion(Vector3::unitZ(),  M_PI/4)), true },
		{   "5.patt", 47.0, Matrix4::makeTransform(Vector3(-76.0/2,        0,       0), Quaternion(Vector3::unitY(), -M_PI/2)*Quaternion(Vector3::unitZ(),  -M_PI/4)), true },
		{  "52.patt", 47.0, Matrix4::makeTransform(Vector3(      0,  -76.0/2,       0), Quaternion(Vector3::unitX(),  M_PI/2)*Quaternion(Vector3::unitZ(),   M_PI/4)), true },
		{  "57.patt", 47.0, Matrix4::makeTransform(Vector3( 76.0/2,        0,       0), Quaternion(Vector3::unitY(),  M_PI/2)*Quaternion(Vector3::unitZ(), 3*M_PI/4)), true },
		{ "260.patt", 47.0, Matrix4::makeTransform(Vector3(      0,   76.0/2,       0), Quaternion(Vector3::unitX(), -M_PI/2)*Quaternion(Vector3::unitZ(),   M_PI/4)), true },
		{  "59.patt", 47.0, Matrix4::makeTransform(Vector3(      0,        0, -76.0/2), Quaternion(Vector3::unitX(),    M_PI)*Quaternion(Vector3::unitZ(),   M_PI/4)), true },

		// NOTE:
		// 26.5 = sqrt(((51/2+24/2)^2)/2)
		// 10.0 = ...
		// 54.736 = atan(sqrt(2))   (aka "magic angle")

		{ "244.patt", 22.0, Matrix4::makeTransform(Vector3(  -23.5,    -23.5,  76.0/2-10.4), Quaternion(Vector3::unitZ(),   -M_PI/4)*Quaternion(Vector3::unitX(), 54.736*M_PI/180)), false },
		{  "65.patt", 22.0, Matrix4::makeTransform(Vector3(   23.5,    -23.5,  76.0/2-10.4), Quaternion(Vector3::unitZ(),    M_PI/4)*Quaternion(Vector3::unitX(), 54.736*M_PI/180)), false },
		{ "929.patt", 22.0, Matrix4::makeTransform(Vector3(   23.5,     23.5,  76.0/2-10.4), Quaternion(Vector3::unitZ(),  3*M_PI/4)*Quaternion(Vector3::unitX(), 54.736*M_PI/180)), false },
		{   "7.patt", 22.0, Matrix4::makeTransform(Vector3(  -23.5,     23.5,  76.0/2-10.4), Quaternion(Vector3::unitZ(), -3*M_PI/4)*Quaternion(Vector3::unitX(), 54.736*M_PI/180)), false },

		{ "540.patt", 22.0, Matrix4::makeTransform(Vector3(   23.5,    -23.5, -76.0/2+10.4), Quaternion(Vector3::unitZ(), -3*M_PI/4)*Quaternion(Vector3::unitX(), (54.736+180)*M_PI/180)), false },
		{ "397.patt", 22.0, Matrix4::makeTransform(Vector3(  -23.5,    -23.5, -76.0/2+10.4), Quaternion(Vector3::unitZ(),  3*M_PI/4)*Quaternion(Vector3::unitX(), (54.736+180)*M_PI/180)), false },
		{   "6.patt", 22.0, Matrix4::makeTransform(Vector3(  -23.5,     23.5, -76.0/2+10.4), Quaternion(Vector3::unitZ(),    M_PI/4)*Quaternion(Vector3::unitX(), (54.736+180)*M_PI/180)), false },
		{   "2.patt", 22.0, Matrix4::makeTransform(Vector3(   23.5,     23.5, -76.0/2+10.4), Quaternion(Vector3::unitZ(),   -M_PI/4)*Quaternion(Vector3::unitX(), (54.736+180)*M_PI/180)), false },
	};

	const std::list<SubMarker> stylus = {
		/* {   "18.patt", 30.0, Matrix4::makeTransform(Vector3(      0,        0,  40.0/2), Quaternion::identity()), true }, */
		/* {   "14.patt", 30.0, Matrix4::makeTransform(Vector3(      0,  -42.0/2,       0), Quaternion(Vector3::unitX(),    M_PI/2)), true }, */
		/* {   "15.patt", 30.0, Matrix4::makeTransform(Vector3(      0,        0, -40.0/2), Quaternion(Vector3::unitX(),    M_PI)),   true }, */
		/* {   "19.patt", 30.0, Matrix4::makeTransform(Vector3(      0,   42.0/2,       0), Quaternion(Vector3::unitX(),  3*M_PI/2)), true }, */

		{   "25.patt", 30.0, Matrix4::makeTransform(Vector3(      0,        0,  13.0), Quaternion::identity()), true },
		/* {   "19.patt", 30.0, Matrix4::makeTransform(Vector3(      0,    -13.0,  -6.0), Quaternion(Vector3::unitX(),  2.146)), true },
		  {   "15.patt", 30.0, Matrix4::makeTransform(Vector3(      0,     13.0,  -6.0), Quaternion(Vector3::unitX(), -2.146)), true }, */
		{   "19.patt", 30.0, Matrix4::makeTransform(Vector3(      0,    -13.0,  -6.0), Quaternion(Vector3::unitX(),  2.09)), true },
		{   "15.patt", 30.0, Matrix4::makeTransform(Vector3(      0,     13.0,  -6.0), Quaternion(Vector3::unitX(), -2.09)), true },

		{   "14.patt", 30.0, Matrix4::makeTransform(Vector3(      0,        0,  18.0), Quaternion::identity()), true },
		{   "24.patt", 30.0, Matrix4::makeTransform(Vector3(      0,     18.0,   0.0), Quaternion(Vector3::unitX(), -M_PI/2)), true },
		{   "26.patt", 30.0, Matrix4::makeTransform(Vector3(      0,    -18.0,   0.0), Quaternion(Vector3::unitX(), M_PI/2)), true },

		/* {   "24.patt", 30.0, Matrix4::makeTransform(Vector3(      0,        0,  12.5), Quaternion::identity()), true }, */

		// {   "26.patt", 30.0, Matrix4::makeTransform(Vector3(     0,     0,   24.0), Quaternion(Vector3::unitX(),  0*72*M_PI/180)), true },
		// {   "15.patt", 30.0, Matrix4::makeTransform(Vector3(     0,   24.7,   5.7), Quaternion(Vector3::unitX(), -1*72*M_PI/180)), true },
		// {   "19.patt", 30.0, Matrix4::makeTransform(Vector3(     0,   14.9, -28.5), Quaternion(Vector3::unitX(), -2*72*M_PI/180)), true },
		// {   "24.patt", 30.0, Matrix4::makeTransform(Vector3(     0,  -14.9, -28.5), Quaternion(Vector3::unitX(), -3*72*M_PI/180)), true },
		// {   "25.patt", 30.0, Matrix4::makeTransform(Vector3(     0,  -24.7,   5.7), Quaternion(Vector3::unitX(), -4*72*M_PI/180)), true },

		// {   "26.patt", 30.0, Matrix4::makeTransform(Vector3(     0,     0,   0), Quaternion(Vector3::unitX(), M_PI)), true },
	};
} // namespace

#endif /* MULTI_MARKER_OBJECTS_H */
