#pragma once
#define Interpolator_h

// ------------------------------------------------------------------------------------------
// Bilinear Interpolation class, taking two x,y values and 4 datapoints to interpolate between
// AUTHORED BY CARLOS DAVEY
// ------------------------------------------------------------------------------------------

#include <stdio.h>

class Interpolator
{
public:

	float interpolate(float xVal, float yVal, float _a11Val, float _a21Val, float _a12Val, float _a22Val)
	{

		a11 = _a11Val;
		a21 = _a21Val - _a11Val;
		a12 = _a12Val - _a11Val;
		a22 = _a22Val + _a11Val - (_a21Val + _a12Val);

		float output = a11 + (a21 * xVal) + (a12 * yVal) + (a22 * xVal * yVal);

		return output;

	}


private:

	float a11;
	float a21;
	float a12;
	float a22;

};