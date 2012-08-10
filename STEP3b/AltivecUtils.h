#ifndef ALTIVEC_TOOLS_H
#define ALTIVEC_TOOLS_H

#define HIGH_PERFORMANCE

#ifdef __SPU__

	#include <simdmath.h>

//	typedef vector unsigned int vec_bint4;

#else
	#include <altivec.h>

	typedef vector bool int vec_bint4;
	
	#ifndef HIGH_PERFORMANCE
		#include <simdmath.h>
	#endif
#endif

static inline vector float splatScalar(const float& scalar)
{
#ifdef __SPU__
	return spu_splats(scalar);
#else
    return vec_splats(scalar);
#endif
}

static inline vector signed int splatScalar(const int& scalar)
{
#ifdef __SPU__
	return spu_splats(scalar);
#else
    return vec_splats(scalar);
#endif
}

static inline vector unsigned int splatScalar(const unsigned int& scalar)
{
#ifdef __SPU__
	return spu_splats(scalar);
#else
	return vec_splats(scalar);
#endif
}

#ifdef __SPU__
static inline vector float negativeZero()
{
	return splatScalar(-0.0f);
}
#else
static vector float ALTIVEC_NEGATIVE_ZERO = splatScalar(-0.0f);

static inline vector float& negativeZero()
{
	return ALTIVEC_NEGATIVE_ZERO;
}
#endif

#ifdef __SPU__
inline vec_bint4 boolZero()
{
	return splatScalar((unsigned int)0);
}
#else
static vec_bint4 ALTIVEC_BOOL_ZERO = (vec_bint4)splatScalar((unsigned int)0);

inline vec_bint4& boolZero()
{
	return ALTIVEC_BOOL_ZERO;
}
#endif

#ifdef __SPU__
static inline vector float vectorOne()
{
	return splatScalar(1.0f);
}
#else
static vector float ALTIVEC_FLOAT_ONE = splatScalar(1.0f);

static inline vector float& vectorOne()
{
    return ALTIVEC_FLOAT_ONE;
}
#endif

#ifdef __SPU__
inline vector float vectorOneHalf()
{
	return splatScalar(0.5f);
}
#else

static vector float ALTIVEC_FLOAT_ONE_HALF = splatScalar(0.5f); 

inline vector float& vectorOneHalf()
{
	return ALTIVEC_FLOAT_ONE_HALF;
}
#endif

#ifdef __SPU__
inline vector float vectorEpsilon()
{
	return splatScalar(0.001f);
}
#else
static vector float ALTIVEC_EPSILON = splatScalar(0.001f);

inline vector float& vectorEpsilon()
{
	return ALTIVEC_EPSILON;
}
#endif

/**
 * Only works for positive exponents.  Negative exponents can multiply by
 * 1^(mantissa)...
 */
inline vector float pow(vector float& expV, vector float& mantissaV)
{
#ifdef HIGH_PERFORMANCE
#ifdef __SPU__
	return powf4(expV, mantissaV);
#else
    vector float logged = vec_loge(vec_abs(expV));
    vector float negZero = negativeZero();
    vector float logMantissa = vec_madd(logged, mantissaV, negZero);
	return vec_expte(logMantissa);
#endif
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
#ifdef __SPU__
	return recipf4(v);
#else
    vector float estimate = vec_re(v);

#ifdef HIGH_PERFORMANCE
	return estimate;
#else
    // one round of Newton-Raphson refinement
	return vec_madd( vec_nmsub(estimate, v, vectorOne()), estimate, estimate);
#endif
#endif
}

inline vector float divide(vector float a, vector float b)
{
#ifdef __SPU__
	return spu_mul(a, reciprocal(b));
#else
	return vec_madd(a, reciprocal(b), negativeZero());
#endif
}

inline vector float rsqrt(vector float v)
{
#ifdef __SPU__
	return rsqrtf4(v);
#else
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
#endif
}

inline vector float sqrt(vector float v)
{
#ifdef __SPU__
	return sqrtf4(v);
#else
	return vec_madd(v, rsqrt(v), negativeZero());
#endif
}

#endif

