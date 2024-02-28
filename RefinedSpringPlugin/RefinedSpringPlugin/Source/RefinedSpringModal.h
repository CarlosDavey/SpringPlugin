#pragma once

//#define RefinedSpringModal_h

// ------------------------------------------------------------------------------------------
// Refined Spring Plugin with multiple models including SSE optimised model and exact scheme
// AUTHORED BY CARLOS DAVEY
// ------------------------------------------------------------------------------------------

#include <cmath>
#include <string>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <math.h>
#include "RealParams.h"
#include "ExpParams.h"
#include "Interpolator.h"
#include "emmintrin.h"


class RefinedSpringModal
{
public:

	/// <summary>
	/// Reset the Sampling Rate SR
	/// </summary>
	/// <param name="_SR">Sample Rate (Hz)</param>
	void setSR(float _SR);
	//=============================================================

	/// <summary>
	/// Set the timestep k from the Samplerate
	/// </summary>
	void setK();
	//=============================================================

	/// <summary>
	/// Set the decay time of the reverb
	/// </summary>
	void setT60(float _T60);
	//=============================================================

	/// <summary>
	/// Import omega values from lookup table
	/// </summary>
	void getDefaultOM();
	//=============================================================

	/// <summary>
	/// Import modal weight values from lookup table
	/// </summary>
	void getDefaultW();
	//=============================================================

	void getDefaultfc();

	/// <summary>
	/// Imports all information from the lookup table by calling the previous 3 functions
	/// </summary>
	void importAll();
	//=============================================================

	/// <summary>
	/// Calculate the frequency dependant loss from physically informed formula
	/// </summary>
	float getLoss(float _freq);
	//=============================================================

	/// <summary>
	/// Calculate the damping coefficients from the imported omega values
	/// </summary>
	void setSig0();
	//=============================================================

	/// <summary>
	/// Calculate the finite-difference scheme coefficients 
	/// </summary>
	void setSchemeCoefficients();
	//=============================================================

	void setSchemeCoefficients_Ex();

	/// <summary>
	/// Update the finite difference scheme and return the current output
	/// </summary>
	/// <returns></returns>
	float schemeUpdate(float _input);
	//=============================================================

	/// <summary>
	/// Scheme update just as "schemeUpdate", but using SSE optimisation
	/// </summary>
	float schemeUpdateSSE(float _input);
	//=============================================================

	/// <summary>
	/// Scheme update just as "schemeUpdateSSE" but using exact scheme
	/// </summary>
	float schemeUpdateSSE_Ex(float _input);
	//=============================================================

	/// <summary>
	/// Setup with all default settings for debugging
	/// </summary>
	void quickStart();
	//=============================================================

	/// <summary>
	/// Same function as "quickstart" but adjusted for exact scheme
	/// </summary>
	void quickStart_Ex();
	//=============================================================

	/// <summary>
	/// print all scheme coefficients for debugging 
	/// </summary>
	void testCoeff();
	//=============================================================

	/// <summary>
	/// Set dark parameter
	/// </summary>
	/// <param name="_isDark"></param>
	void setDark(bool _isDark);
	//=============================================================

	/// <summary>
	/// Updates the decay coefficient in runtime
	/// </summary>
	/// <param name="decayParam"></param>
	void updateT60(float decayParam);
	//=============================================================

	/// <summary>
	/// Select correct 4 datasets to bilinearly interpolate between, before actually performing the interpolation for frequency, gain factor, modal weight and cutoff frequency
	/// </summary>
	/// <param name="rParam">Radius Parameter</param>
	/// <param name="lParam">Length Parameter</param>
	void updateParams(float rParam, float lParam);

	/// <summary>
	/// Update the value of the dark parameter boolean
	/// </summary>
	/// <param name="darkParam"></param>
	void updateDark(bool darkParam);

	/// <summary>
	/// Apply the "dark" effect by damping all modes above cutoff to 0, if the dark parameter is true
	/// </summary>
	void applyDark();

	/// <summary>
	/// Recalculate coeffificients and damping constants if parameters changed by the user.
	/// </summary>
	void updateCoefficients();

	/// <summary>
	/// Same function as "updateCoefficients" but adjusted for exact scheme
	/// </summary>
	void updateCoefficients_Ex();
	//=============================================================

	/// <summary>
	/// Updates real parameter when called
	/// </summary>
	/// <param name="realParam"></param>
	void updateReal(bool realParam);
	//=============================================================

private:

	// GLOBAL PARAMS
	//=============================================================
	float SR = 44100;						// Global Sampling Rate (Hz)
	float k;								// Timestep (s)

	float Xin = 0.1f;						// Location of input signal writing (%)
	float Xout = 0.9f;						// Location of output signal reading (%)

	float T60 = 1.0f;						// Decay time (s)

	float L = 20.0f;						// Length of spring

	bool isDark = false;					// Control which damping profile is used
	bool isReal = true;						// Control if real or experimental parameters are used

	float M = 10.0f;						// Scaling Mass 
	float M_1 = 1 / M;						// Inverted Scaling Mass
	//=============================================================

	// IMPORTED MODES DATA
	//=============================================================
	static constexpr const int maxOmN = 4000;

	int OM_N = maxOmN;						// Number of resonant frequencies

	float OM[maxOmN];						// Values of resonant frequencies

	float W[maxOmN];						// Values of modal weightings

	float sig0[maxOmN];						// Decay Coefficients

	int fc;									// Cutoff frequency

	float gainFactor;						//  Gain factor for normalisation
	//=============================================================


	// VECTORS
	//=============================================================

	alignas(32) float pMem[maxOmN] = { 0 };					// State Arrays
	alignas(32) float p1Mem[maxOmN] = { 0 };				//
	alignas(32) float p2Mem[maxOmN] = { 0 };				//	

	alignas(32) float* p = pMem;							// State array referencing 
	alignas(32) float* p1 = p1Mem;							//
	alignas(32) float* p2 = p2Mem;							//
	alignas(32) float* dummyPtr = dummyPtrMem;				//

	float F;												// Scheme Coefficients
	alignas(32) float H[maxOmN];							//
	alignas(32) float G[maxOmN];							//
	alignas(32) float J[maxOmN];							//
	alignas(32) float dummyPtrMem[maxOmN] = { 0 };			// Dummy pointer used for state updates

	alignas(32) float A[maxOmN];							// Exact Scheme coefficients
	alignas(32) float B[maxOmN];							//

	//=============================================================

	const float pi = 3.14159f;				// In-house pi value
	Interpolator Interp;					// Initialise interpolator function
};