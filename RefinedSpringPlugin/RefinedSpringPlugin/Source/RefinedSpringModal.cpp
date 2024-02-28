
//#define RefinedSpringModal_cpp

//#define _USE_MATH_DEFINES

#include "RefinedSpringModal.h"

void RefinedSpringModal::setSR(float _SR)
{
	SR = _SR;
}

void RefinedSpringModal::setK()
{
	k = 1 / SR;
}


void RefinedSpringModal::setT60(float _T60)
{
	T60 = _T60;
}


void RefinedSpringModal::getDefaultOM()
{

	for (int i = 0; i < OM_N; i++)
	{
		OM[i] = R_L1r1_Om[i];
	}
}

void RefinedSpringModal::getDefaultW()
{
	for (int i = 0; i < OM_N; i++)
	{
		W[i] = R_L1r1_W[i] * R_L1r1_gf;
	}

}

void RefinedSpringModal::getDefaultfc()
{
	fc = R_L1r1_fc;
}

void RefinedSpringModal::importAll()
{
	getDefaultOM();
	getDefaultW();
	getDefaultfc();
}

float RefinedSpringModal::getLoss(float _freq)
{
	if (isDark)
	{
		float coef1 = exp((-0.4525f * log(_freq)) + 4.6417f);

		return coef1;
	}
	else
	{
		float coef2 = exp((-3.0f * log(_freq)) + 4.6417f) + (0.001f * _freq);

		return coef2;
	}

}

void RefinedSpringModal::setSig0()
{
	for (int i = 0; i < OM_N; i++)
	{
		float T30 = getLoss(OM[i] / 2 / pi);
		sig0[i] = 6.0f * T60 * log(10.0f) / T30;
	}
}

void RefinedSpringModal::setSchemeCoefficients()
{
	for (int i = 0; i < OM_N; i++)
	{
		F = 1 + sig0[i] * k / 2;
		H[i] = ((sig0[i] * k / 2) - 1) / F;
		G[i] = (2.0f - (pow(k, 2.0f) * pow(OM[i], 2.0f))) / F;
		J[i] = (pow(k, 2.0f) / F) * M_1 * W[i];
	}
}

void RefinedSpringModal::setSchemeCoefficients_Ex()
{

	for (int i = 0; i < OM_N; i++)
	{
		F = 1 + sig0[i] * k / 2;
		A[i] = 2 * exp(-sig0[i] * k) * std::cosf(OM[i] * k);
		B[i] = -exp(-2 * sig0[i] * k);
		J[i] = (pow(k, 2.0f) / F) * M_1 * W[i];
	}
}

float RefinedSpringModal::schemeUpdate(float _input)
{
	float output = 0.0f;

	for (int i = 0; i < OM_N; i++)
	{
		p[i] = (G[i] * p1[i]) + (H[i] * p2[i]) + (J[i] * _input);
	}

	for (int i = 0; i < OM_N; i++)
	{
		output += p[i];
	}

	dummyPtr = p2;
	p2 = p1;
	p1 = p;
	p = dummyPtr;

	return output;
}

float RefinedSpringModal::schemeUpdateSSE(float _input)
{

	__m128 inv = _mm_set1_ps(_input);

	for (int i = 0; i < OM_N; i += 4)
	{
		__m128 pv = _mm_set1_ps(0.0f);
		__m128 p1v = _mm_load_ps(&p1[i]);
		__m128 p2v = _mm_load_ps(&p2[i]);

		__m128 Gv = _mm_load_ps(&G[i]);
		__m128 Hv = _mm_load_ps(&H[i]);
		__m128 Jv = _mm_load_ps(&J[i]);
#
		__m128 Gp1 = _mm_mul_ps(Gv, p1v);
		__m128 Hp2 = _mm_mul_ps(Hv, p2v);
		__m128 Jinv = _mm_mul_ps(Jv, inv);

		pv = _mm_add_ps(pv, Gp1);
		pv = _mm_add_ps(pv, Hp2);
		pv = _mm_add_ps(pv, Jinv);

		_mm_store_ps(&p[i], pv);

	}

	__m128 output = _mm_set1_ps(0.0f);

	for (int i = 0; i < OM_N; i += 4)
	{
		__m128 pv = _mm_load_ps(&p[i]);

		output = _mm_add_ps(output, pv);
	}

	float vs[4];
	_mm_store_ps(vs, output);

	float outVal = vs[0] + vs[1] + vs[2] + vs[3];

	dummyPtr = p2;
	p2 = p1;
	p1 = p;
	p = dummyPtr;

	return outVal;
}

float RefinedSpringModal::schemeUpdateSSE_Ex(float _input)
{

	__m128 inv = _mm_set1_ps(_input);

	for (int i = 0; i < OM_N; i += 4)
	{
		__m128 pv = _mm_set1_ps(0.0f);
		__m128 p1v = _mm_load_ps(&p1[i]);
		__m128 p2v = _mm_load_ps(&p2[i]);

		__m128 Av = _mm_load_ps(&A[i]);
		__m128 Bv = _mm_load_ps(&B[i]);
		__m128 Jv = _mm_load_ps(&J[i]);
#
		__m128 Ap1 = _mm_mul_ps(Av, p1v);
		__m128 Bp2 = _mm_mul_ps(Bv, p2v);
		__m128 Jinv = _mm_mul_ps(Jv, inv);

		pv = _mm_add_ps(pv, Ap1);
		pv = _mm_add_ps(pv, Bp2);
		pv = _mm_add_ps(pv, Jinv);

		_mm_store_ps(&p[i], pv);

	}

	__m128 output = _mm_set1_ps(0.0f);

	for (int i = 0; i < OM_N; i += 4)
	{
		__m128 pv = _mm_load_ps(&p[i]);

		output = _mm_add_ps(output, pv);
	}

	float vs[4];
	_mm_store_ps(vs, output);

	float outVal = vs[0] + vs[1] + vs[2] + vs[3];

	dummyPtr = p2;
	p2 = p1;
	p1 = p;
	p = dummyPtr;

	return outVal;
}


void RefinedSpringModal::quickStart()
{
	setK();
	importAll();
	setSig0();
	setSchemeCoefficients();
}

void RefinedSpringModal::quickStart_Ex()
{
	setK();
	importAll();
	setSig0();
	updateParams(0.0f, 0.0f);
	setSchemeCoefficients_Ex();
}


void RefinedSpringModal::testCoeff()
{
	for (int i = 0; i < OM_N; i++)
	{
		std::cout << J[i] << "\n";
	}
}

void RefinedSpringModal::setDark(bool _isDark)
{
	isDark = _isDark;
}


void RefinedSpringModal::updateT60(float decayParam)
{
	T60 = decayParam;
}

void RefinedSpringModal::updateDark(bool darkParam)
{
	isDark = darkParam;
}

void RefinedSpringModal::updateReal(bool realParam)
{
	isReal = realParam;

}

void RefinedSpringModal::updateParams(float rParam, float lParam)
{
	float rSel = trunc(rParam * 3) + 1;
	float lSel = trunc(lParam * 3) + 1;



	if (rSel == 4)
	{
		rSel = 3;
	}

	if (lSel == 4)
	{
		lSel = 3;
	}

	if (rParam > 0.33f && rParam <= 0.66)
	{
		rParam = rParam - 0.34f;
	}

	if (lParam > 0.33f && lParam <= 0.66)
	{
		lParam = lParam - 0.34f;
	}

	if (rParam > 0.66f)
	{
		rParam = rParam - 0.67f;
	}

	if (lParam > 0.66f)
	{
		lParam = lParam - 0.67f;
	}

	rParam = rParam * 3.0f;
	lParam = lParam * 3.0f;

	if (isReal == true)
	{
		if (rSel == 1 && lSel == 1)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, R_L1r1_Om[i], R_L2r1_Om[i], R_L1r2_Om[i], R_L2r2_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L1r1_gf, R_L2r1_gf, R_L1r2_gf, R_L2r2_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L1r1_gf_br, R_L2r1_gf_br, R_L1r2_gf_br, R_L2r2_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, R_L1r1_W[i], R_L2r1_W[i], R_L1r2_W[i], R_L2r2_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, R_L1r1_fc, R_L2r1_fc, R_L1r2_fc, R_L2r2_fc));
			}
		}

		if (rSel == 1 && lSel == 2)
		{
			for (int i = 0; i < OM_N; i++)
			{
				OM[i] = Interp.interpolate(lParam, rParam, R_L2r1_Om[i], R_L3r1_Om[i], R_L2r2_Om[i], R_L3r2_Om[i]);

				gainFactor = Interp.interpolate(lParam, rParam, R_L2r1_gf, R_L3r1_gf, R_L2r2_gf, R_L3r2_gf);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L2r1_gf, R_L3r1_gf, R_L2r2_gf, R_L3r2_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L2r1_gf_br, R_L3r1_gf_br, R_L2r2_gf_br, R_L3r2_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, R_L2r1_W[i], R_L3r1_W[i], R_L2r2_W[i], R_L3r2_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, R_L2r1_fc, R_L3r1_fc, R_L2r2_fc, R_L3r2_fc));
			}
		}

		if (rSel == 1 && lSel == 3)
		{
			for (int i = 0; i < OM_N; i++)
			{
				OM[i] = Interp.interpolate(lParam, rParam, R_L3r1_Om[i], R_L4r1_Om[i], R_L3r2_Om[i], R_L4r2_Om[i]);

				gainFactor = Interp.interpolate(lParam, rParam, R_L3r1_gf, R_L4r1_gf, R_L3r2_gf, R_L4r2_gf);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L3r1_gf, R_L4r1_gf, R_L3r2_gf, R_L4r2_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L3r1_gf_br, R_L4r1_gf_br, R_L3r2_gf_br, R_L4r2_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, R_L3r1_W[i], R_L4r1_W[i], R_L3r2_W[i], R_L4r2_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, R_L3r1_fc, R_L4r1_fc, R_L3r2_fc, R_L4r2_fc));
			}
		}

		if (rSel == 2 && lSel == 1)
		{
			for (int i = 0; i < OM_N; i++)
			{
				OM[i] = Interp.interpolate(lParam, rParam, R_L1r2_Om[i], R_L2r2_Om[i], R_L1r3_Om[i], R_L2r3_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L1r2_gf, R_L2r2_gf, R_L1r3_gf, R_L2r3_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L1r2_gf_br, R_L2r2_gf_br, R_L1r3_gf_br, R_L2r3_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, R_L1r2_W[i], R_L2r2_W[i], R_L1r3_W[i], R_L2r3_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, R_L1r2_fc, R_L2r2_fc, R_L1r3_fc, R_L2r3_fc));
			}
		}

		if (rSel == 2 && lSel == 2)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, R_L2r2_Om[i], R_L3r2_Om[i], R_L2r3_Om[i], R_L3r3_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L2r2_gf, R_L3r2_gf, R_L2r3_gf, R_L3r3_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L2r2_gf_br, R_L3r2_gf_br, R_L2r3_gf_br, R_L3r3_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, R_L2r2_W[i], R_L3r2_W[i], R_L2r3_W[i], R_L3r3_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, R_L2r2_fc, R_L3r2_fc, R_L2r3_fc, R_L3r3_fc));
			}
		}

		if (rSel == 2 && lSel == 3)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, R_L3r2_Om[i], R_L4r2_Om[i], R_L3r3_Om[i], R_L4r3_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L3r2_gf, R_L4r2_gf, R_L3r3_gf, R_L4r3_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L3r2_gf_br, R_L4r2_gf_br, R_L3r3_gf_br, R_L4r3_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, R_L3r2_W[i], R_L4r2_W[i], R_L3r3_W[i], R_L4r3_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, R_L3r2_fc, R_L4r2_fc, R_L3r3_fc, R_L4r3_fc));
			}
		}

		if (rSel == 3 && lSel == 1)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, R_L1r3_Om[i], R_L2r3_Om[i], R_L1r4_Om[i], R_L2r4_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L1r3_gf, R_L2r3_gf, R_L1r4_gf, R_L2r4_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L1r3_gf_br, R_L2r3_gf_br, R_L1r4_gf_br, R_L2r4_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, R_L1r3_W[i], R_L2r3_W[i], R_L1r4_W[i], R_L2r4_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, R_L1r3_fc, R_L2r3_fc, R_L1r4_fc, R_L2r4_fc));
			}
		}

		if (rSel == 3 && lSel == 2)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, R_L2r3_Om[i], R_L3r3_Om[i], R_L2r4_Om[i], R_L3r4_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L2r3_gf, R_L3r3_gf, R_L2r4_gf, R_L3r4_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L2r3_gf_br, R_L3r3_gf_br, R_L2r4_gf_br, R_L3r4_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, R_L2r3_W[i], R_L3r3_W[i], R_L2r4_W[i], R_L3r4_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, R_L2r3_fc, R_L3r3_fc, R_L2r4_fc, R_L3r4_fc));
			}
		}

		if (rSel == 3 && lSel == 3)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, R_L3r3_Om[i], R_L4r3_Om[i], R_L3r4_Om[i], R_L4r4_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L3r3_gf, R_L4r3_gf, R_L3r4_gf, R_L4r4_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, R_L3r3_gf_br, R_L4r3_gf_br, R_L3r4_gf_br, R_L4r4_gf_br);
				}


				W[i] = Interp.interpolate(lParam, rParam, R_L3r3_W[i], R_L4r3_W[i], R_L3r4_W[i], R_L4r4_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, R_L3r3_fc, R_L4r3_fc, R_L3r4_fc, R_L4r4_fc));
			}
		}
	}

	else
	{
		if (rSel == 1 && lSel == 1)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, E_L1r1_Om[i], E_L2r1_Om[i], E_L1r2_Om[i], E_L2r2_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L1r1_gf, E_L2r1_gf, E_L1r2_gf, E_L2r2_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L1r1_gf_br, E_L2r1_gf_br, E_L1r2_gf_br, E_L2r2_gf_br) * (pow(std::fabsf(1.06f - 2 * rParam), 8) + 0.3);
				}

				W[i] = Interp.interpolate(lParam, rParam, E_L1r1_W[i], E_L2r1_W[i], E_L1r2_W[i], E_L2r2_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, E_L1r1_fc, E_L2r1_fc, E_L1r2_fc, E_L2r2_fc));
			}
		}

		if (rSel == 1 && lSel == 2)
		{
			for (int i = 0; i < OM_N; i++)
			{
				OM[i] = Interp.interpolate(lParam, rParam, E_L2r1_Om[i], E_L3r1_Om[i], E_L2r2_Om[i], E_L3r2_Om[i]);

				gainFactor = Interp.interpolate(lParam, rParam, E_L2r1_gf, E_L3r1_gf, E_L2r2_gf, E_L3r2_gf);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L2r1_gf, E_L3r1_gf, E_L2r2_gf, E_L3r2_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L2r1_gf_br, E_L3r1_gf_br, E_L2r2_gf_br, E_L3r2_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, E_L2r1_W[i], E_L3r1_W[i], E_L2r2_W[i], E_L3r2_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, E_L2r1_fc, E_L3r1_fc, E_L2r2_fc, E_L3r2_fc));
			}
		}

		if (rSel == 1 && lSel == 3)
		{
			for (int i = 0; i < OM_N; i++)
			{
				OM[i] = Interp.interpolate(lParam, rParam, E_L3r1_Om[i], E_L4r1_Om[i], E_L3r2_Om[i], E_L4r2_Om[i]);

				gainFactor = Interp.interpolate(lParam, rParam, E_L3r1_gf, E_L4r1_gf, E_L3r2_gf, E_L4r2_gf);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L3r1_gf, E_L4r1_gf, E_L3r2_gf, E_L4r2_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L3r1_gf_br, E_L4r1_gf_br, E_L3r2_gf_br, E_L4r2_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, E_L3r1_W[i], E_L4r1_W[i], E_L3r2_W[i], E_L4r2_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, E_L3r1_fc, E_L4r1_fc, E_L3r2_fc, E_L4r2_fc));
			}
		}

		if (rSel == 2 && lSel == 1)
		{
			for (int i = 0; i < OM_N; i++)
			{
				OM[i] = Interp.interpolate(lParam, rParam, E_L1r2_Om[i], E_L2r2_Om[i], E_L1r3_Om[i], E_L2r3_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L1r2_gf, E_L2r2_gf, E_L1r3_gf, E_L2r3_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L1r2_gf_br, E_L2r2_gf_br, E_L1r3_gf_br, E_L2r3_gf_br) * (pow(std::fabsf(1.06f - 2 * rParam), 8) + 0.05) * 0.2;
				}

				W[i] = Interp.interpolate(lParam, rParam, E_L1r2_W[i], E_L2r2_W[i], E_L1r3_W[i], E_L2r3_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, E_L1r2_fc, E_L2r2_fc, E_L1r3_fc, E_L2r3_fc));
			}
		}

		if (rSel == 2 && lSel == 2)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, E_L2r2_Om[i], E_L3r2_Om[i], E_L2r3_Om[i], E_L3r3_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L2r2_gf, E_L3r2_gf, E_L2r3_gf, E_L3r3_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L2r2_gf_br, E_L3r2_gf_br, E_L2r3_gf_br, E_L3r3_gf_br) * (pow(std::fabsf(0.8f - (2 * rParam * lParam)), 8) + 0.05);
				}

				W[i] = Interp.interpolate(lParam, rParam, E_L2r2_W[i], E_L3r2_W[i], E_L2r3_W[i], E_L3r3_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, E_L2r2_fc, E_L3r2_fc, E_L2r3_fc, E_L3r3_fc));
			}
		}

		if (rSel == 2 && lSel == 3)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, E_L3r2_Om[i], E_L4r2_Om[i], E_L3r3_Om[i], E_L4r3_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L3r2_gf, E_L4r2_gf, E_L3r3_gf, E_L4r3_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L3r2_gf_br, E_L4r2_gf_br, E_L3r3_gf_br, E_L4r3_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, E_L3r2_W[i], E_L4r2_W[i], E_L3r3_W[i], E_L4r3_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, E_L3r2_fc, E_L4r2_fc, E_L3r3_fc, E_L4r3_fc));
			}
		}

		if (rSel == 3 && lSel == 1)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, E_L1r3_Om[i], E_L2r3_Om[i], E_L1r4_Om[i], E_L2r4_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L1r3_gf, E_L2r3_gf, E_L1r4_gf, E_L2r4_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L1r3_gf_br, E_L2r3_gf_br, E_L1r4_gf_br, E_L2r4_gf_br);
				}

				W[i] = Interp.interpolate(lParam, rParam, E_L1r3_W[i], E_L2r3_W[i], E_L1r4_W[i], E_L2r4_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, E_L1r3_fc, E_L2r3_fc, E_L1r4_fc, E_L2r4_fc));
			}
		}

		if (rSel == 3 && lSel == 2)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, E_L2r3_Om[i], E_L3r3_Om[i], E_L2r4_Om[i], E_L3r4_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L2r3_gf, E_L3r3_gf, E_L2r4_gf, E_L3r4_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L2r3_gf_br, E_L3r3_gf_br, E_L2r4_gf_br, E_L3r4_gf_br) * (pow(std::fabsf(0.8f - (2 * rParam * lParam)), 8) + 0.05) * 0.5;
				}

				W[i] = Interp.interpolate(lParam, rParam, E_L2r3_W[i], E_L3r3_W[i], E_L2r4_W[i], E_L3r4_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, E_L2r3_fc, E_L3r3_fc, E_L2r4_fc, E_L3r4_fc));
			}
		}

		if (rSel == 3 && lSel == 3)
		{
			for (int i = 0; i < OM_N; i++)
			{

				OM[i] = Interp.interpolate(lParam, rParam, E_L3r3_Om[i], E_L4r3_Om[i], E_L3r4_Om[i], E_L4r4_Om[i]);

				if (isDark)
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L3r3_gf, E_L4r3_gf, E_L3r4_gf, E_L4r4_gf);
				}
				else
				{
					gainFactor = Interp.interpolate(lParam, rParam, E_L3r3_gf_br, E_L4r3_gf_br, E_L3r4_gf_br, E_L4r4_gf_br) * (pow(std::fabsf(0.8f - (2 * rParam * lParam)), 8) + 0.2) * 1.5f;
				}


				W[i] = Interp.interpolate(lParam, rParam, E_L3r3_W[i], E_L4r3_W[i], E_L3r4_W[i], E_L4r4_W[i]) * gainFactor;

				fc = (int)round(Interp.interpolate(lParam, rParam, E_L3r3_fc, E_L4r3_fc, E_L3r4_fc, E_L4r4_fc));
			}
		}

	}


}

void RefinedSpringModal::updateCoefficients()
{
	setSig0();
	setSchemeCoefficients();
}

void RefinedSpringModal::updateCoefficients_Ex()
{
	setSig0();
	setSchemeCoefficients_Ex();
}