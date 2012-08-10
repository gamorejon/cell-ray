#ifndef ALTIVEC_TOOLS_H
#define ALTIVEC_TOOLS_H

#define HIGH_PERFORMANCE

#ifndef HIGH_PERFORMANCE
#include <simdmath.h>
#endif

static inline vector float splatScalar(const float& scalar)
{
    return vec_splats(scalar);
}

static inline vector int splatScalar(const int& scalar)
{
    return vec_splats(scalar);
}

static vector float ALTIVEC_NEGATIVE_ZERO = splatScalar(-0.0f);

static inline vector float negativeZero()
{
	return ALTIVEC_NEGATIVE_ZERO;
}

static vector bool int ALTIVEC_BOOL_ZERO = (vector bool int)vec_splats(0);

inline vector bool int& boolZero()
{
	return ALTIVEC_BOOL_ZERO;
}

static vector float ALTIVEC_FLOAT_ONE = splatScalar(1.0f);

inline vector float& vectorOne()
{
    return ALTIVEC_FLOAT_ONE;
}

static vector float ALTIVEC_FLOAT_ONE_HALF = splatScalar(0.5f); 

inline vector float& vectorOneHalf()
{
	return ALTIVEC_FLOAT_ONE_HALF;
}

static vector float ALTIVEC_EPSILON = splatScalar(0.001f);

inline vector float& vectorEpsilon()
{
	return ALTIVEC_EPSILON;
}

/**
 * Only works for positive exponents.  Negative exponents can multiply by
 * 1^(mantissa)...
 */
inline vector float pow(vector float& expV, vector float& mantissaV)
{
#ifdef HIGH_PERFORMANCE
    vector float logged = vec_loge(vec_abs(expV));
    vector float negZero = negativeZero();
    vector float logMantissa = vec_madd(logged, mantissaV, negZero);
	return vec_expte(logMantissa);
#else

// No Newton-Raphson refinement possible, use pow4f from <simdmath.h>
	return pow4f(mantissaV);
#endif
}

inline vector float pow(vector float& exp, const float& _mantissa)
{
	vector float mantissa = splatScalar(_mantissa);

	return pow(exp, mantissa);
}

inline vector float reciprocal(vector float v)
{
    vector float estimate = vec_re(v);

#ifdef HIGH_PERFORMANCE
	return estimate;
#else
    // one round of Newton-Raphson refinement
	return vec_madd( vec_nmsub(estimate, v, vectorOne()), estimate, estimate);
#endif
}

inline vector float divide(vector float a, vector float b)
{
	return vec_madd(a, reciprocal(b), negativeZero());
}

inline vector float rsqrt(vector float v)
{
	vector float estimate = vec_rsqrte(v);

#ifdef HIGH_PERFORMANCE
	return estimate;
#else
	vector float zero = negativeZero();
	vector float oneHalf = vectorOneHalf();
	vector float one = vectorOne();
	vector float estimateSquared = vec_madd( estimate, estimate, zero );
	vector float halfEstimate = vec_madd( estimate, oneHalf, zero );
	return vec_madd( vec_nmsub( v, estimateSquared, one ), halfEstimate, 
		estimate );
#endif
}

inline vector float sqrt(vector float v)
{
	return vec_madd(v, rsqrt(v), negativeZero());
}

#endif

