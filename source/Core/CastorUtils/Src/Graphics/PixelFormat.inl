#include <cstring>
#include "UnsupportedFormatException.hpp"
#include "Miscellaneous/StringUtils.hpp"
#include "Exception/Assertion.hpp"

#if CASTOR_USE_SSE2
#	include <emmintrin.h>
#endif

#if defined( max )
#	undef max
#endif

#if defined( min )
#	undef min
#endif

namespace Castor
{
	static inline void HalfToFloat( float & target, uint16_t const * source )
	{
		uint32_t * xp = ( uint32_t * )&target; // Type pun output as an unsigned 32-bit int
		uint16_t h = *source;

		if ( ( h & 0x7FFFu ) == 0 )
		{
			// Signed zero
			*xp = ( ( uint32_t )h ) << 16;  // Return the signed zero
		}
		else
		{
			// Not zero
			uint16_t hs = h & 0x8000u;  // Pick off sign bit
			uint16_t he = h & 0x7C00u;  // Pick off exponent bits
			uint16_t hm = h & 0x03FFu;  // Pick off mantissa bits

			if ( he == 0 )
			{
				// Denormal will convert to normalized
				int32_t e = -1; // The following loop figures out how much extra to adjust the exponent

				do
				{
					e++;
					hm <<= 1;
				}
				while ( ( hm & 0x0400u ) == 0 ); // Shift until leading bit overflows into exponent bit

				uint32_t xs = ( ( uint32_t )hs ) << 16; // Sign bit
				int32_t xes = ( ( int32_t )( he >> 10 ) ) - 15 + 127 - e; // Exponent unbias the halfp, then bias the single
				uint32_t xe = ( uint32_t )( xes << 23 ); // Exponent
				uint32_t xm = ( ( uint32_t )( hm & 0x03FFu ) ) << 13; // Mantissa
				*xp = ( xs | xe | xm ); // Combine sign bit, exponent bits, and mantissa bits
			}
			else if ( he == 0x7C00u )
			{
				// Inf or NaN (all the exponent bits are set)
				if ( hm == 0 )
				{
					// If mantissa is zero ...
					*xp = ( ( ( uint32_t )hs ) << 16 ) | ( ( uint32_t )0x7F800000u ); // Signed Inf
				}
				else
				{
					*xp = ( uint32_t )0xFFC00000u; // NaN, only 1st mantissa bit set
				}
			}
			else
			{
				// Normalized number
				uint32_t xs = ( ( uint32_t )hs ) << 16; // Sign bit
				int32_t xes = ( ( int32_t )( he >> 10 ) ) - 15 + 127; // Exponent unbias the halfp, then bias the single
				uint32_t xe = ( uint32_t )( xes << 23 ); // Exponent
				uint32_t xm = ( ( uint32_t )hm ) << 13; // Mantissa
				*xp = ( xs | xe | xm ); // Combine sign bit, exponent bits, and mantissa bits
			}
		}
	}

	static inline void FloatToHalf( uint16_t * target, float source )
	{
		uint16_t * hp = target; // Type pun output as an unsigned 16-bit int
		uint32_t x = *reinterpret_cast< uint32_t * >( &source ); // Type pun input as an unsigned 32-bit int

		if ( ( x & 0x7FFFFFFFu ) == 0 )
		{
			// Signed zero
			*hp = uint16_t( x >> 16 );  // Return the signed zero
		}
		else
		{
			// Not zero
			uint32_t xs = x & 0x80000000u;  // Pick off sign bit
			uint32_t xe = x & 0x7F800000u;  // Pick off exponent bits
			uint32_t xm = x & 0x007FFFFFu;  // Pick off mantissa bits

			if ( xe == 0 )
			{
				// Denormal will underflow, return a signed zero
				*hp = uint16_t( xs >> 16 );
			}
			else if ( xe == 0x7F800000u )
			{
				// Inf or NaN (all the exponent bits are set)
				if ( xm == 0 )
				{
					// If mantissa is zero ...
					*hp = uint16_t( ( xs >> 16 ) | 0x7C00u ); // Signed Inf
				}
				else
				{
					*hp = uint16_t( 0xFE00u ); // NaN, only 1st mantissa bit set
				}
			}
			else
			{
				// Normalized number
				uint16_t hs( xs >> 16 ); // Sign bit
				int32_t hes( ( ( int )( xe >> 23 ) ) - 127 + 15 ); // Exponent unbias the single, then bias the halfp
				uint16_t hm{};

				if ( hes >= 0x1F )
				{
					// Overflow
					*hp = uint16_t( ( xs >> 16 ) | 0x7C00u ); // Signed Inf
				}
				else if ( hes <= 0 )
				{
					// Underflow
					if ( ( 14 - hes ) > 24 )
					{
						// Mantissa shifted all the way off & no rounding possibility
						hm = 0u;  // Set mantissa to zero
					}
					else
					{
						xm |= 0x00800000u;  // Add the hidden leading bit
						hm = uint16_t( xm >> ( 14 - hes ) ); // Mantissa

						if ( ( xm >> ( 13 - hes ) ) & 0x00000001u ) // Check for rounding
						{
							hm += ( uint16_t )1u; // Round, might overflow into exp bit, but this is OK
						}
					}

					*hp = ( hs | hm ); // Combine sign bit and mantissa bits, biased exponent is zero
				}
				else
				{
					uint16_t he( hes << 10 ); // Exponent
					uint16_t hm( xm >> 13 ); // Mantissa

					if ( xm & 0x00001000u ) // Check for rounding
					{
						*hp = ( hs | he | hm ) + ( uint16_t )1u; // Round, might overflow to inf, this is OK
					}
					else
					{
						*hp = ( hs | he | hm );  // No rounding
					}
				}
			}
		}
	}

	//!\~english	Specialisation for PixelFormat::eL8
	//!\~french		Spécialisation pour PixelFormat::eL8
	template<>
	struct component< PixelFormat::eL8 >
	{
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return p_pSrc[0];
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return p_pSrc[0];
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return p_pSrc[0];
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return p_pSrc[0];
		}
		static inline uint8_t A8( uint8_t const * )
		{
			return 0xFF;
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			p_pSrc[0] = p_val;
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			p_pSrc[0] = p_val;
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			p_pSrc[0] = p_val;
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			p_pSrc[0] = p_val;
		}
		static inline void A8( uint8_t *, uint8_t )
		{
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t *, float )
		{
		}
	};
	//!\~english	Specialisation for PixelFormat::eL16F32F
	//!\~french		Spécialisation pour PixelFormat::eL16F32F
	template<>
	struct component< PixelFormat::eL16F32F >
	{
#define src ( *reinterpret_cast< float const * >( p_pSrc ) )
#define dst ( *reinterpret_cast< float * >( p_pSrc ) )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( R32F( p_pSrc ) * 255 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( G32F( p_pSrc ) * 255 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( B32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( A32F( p_pSrc ) * 255 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float A32F( uint8_t const * )
		{
			return 1.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			G32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			B32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void A8( uint8_t *, uint8_t pal )
		{
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void A32F( uint8_t *, float )
		{
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eL32F
	//!\~french		Spécialisation pour PixelFormat::eL32F
	template<>
	struct component< PixelFormat::eL32F >
	{
#define src ( *reinterpret_cast< float const * >( p_pSrc ) )
#define dst ( *reinterpret_cast< float * >( p_pSrc ) )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( R32F( p_pSrc ) * 255 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( G32F( p_pSrc ) * 255 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( B32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( A32F( p_pSrc ) * 255 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float A32F( uint8_t const * )
		{
			return 1.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			G32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			B32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void A8( uint8_t *, uint8_t pal )
		{
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void A32F( uint8_t *, float )
		{
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eA8L8
	//!\~french		Spécialisation pour PixelFormat::eA8L8
	template<>
	struct component< PixelFormat::eA8L8 >
	{
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return p_pSrc[0];
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return p_pSrc[0];
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return p_pSrc[0];
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return p_pSrc[0];
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return p_pSrc[1];
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			p_pSrc[0] = p_val;
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			p_pSrc[0] = p_val;
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			p_pSrc[0] = p_val;
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			p_pSrc[0] = p_val;
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			p_pSrc[1] = p_val;
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			A8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
	};
	//!\~english	Specialisation for PixelFormat::eAL16F32F
	//!\~french		Spécialisation pour PixelFormat::eAL16F32F
	template<>
	struct component< PixelFormat::eAL16F32F >
	{
#define src reinterpret_cast< float const * >( p_pSrc )
#define dst reinterpret_cast< float * >( p_pSrc )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( R32F( p_pSrc ) * 255 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( G32F( p_pSrc ) * 255 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( B32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( A32F( p_pSrc ) * 255 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return src[0];
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return src[0];
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return src[0];
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return src[0];
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return src[1];
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			G32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			B32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			A32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			dst[0] = p_val;
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			dst[0] = p_val;
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			dst[0] = p_val;
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			dst[0] = p_val;
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			dst[1] = p_val;
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eAL32F
	//!\~french		Spécialisation pour PixelFormat::eAL32F
	template<>
	struct component< PixelFormat::eAL32F >
	{
#define src reinterpret_cast< float const * >( p_pSrc )
#define dst reinterpret_cast< float * >( p_pSrc )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( R32F( p_pSrc ) * 255 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( G32F( p_pSrc ) * 255 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( B32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( A32F( p_pSrc ) * 255 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return src[0];
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return src[0];
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return src[0];
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return src[0];
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return src[1];
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			G32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			B32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			A32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			dst[0] = p_val;
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			dst[0] = p_val;
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			dst[0] = p_val;
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			dst[0] = p_val;
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			dst[1] = p_val;
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eA1R5G5B5
	//!\~french		Spécialisation pour PixelFormat::eA1R5G5B5
	template<>
	struct component< PixelFormat::eA1R5G5B5 >
	{
#define src ( *reinterpret_cast< uint16_t const * >( p_pSrc ) )
#define dst ( *reinterpret_cast< uint16_t * >( p_pSrc ) )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( R8( p_pSrc ) * 0.30 + G8( p_pSrc ) * 0.59 + B8( p_pSrc ) * 0.11 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src & 0x8000 ) ? 0xFF : 0x00 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src & 0x7C00 ) >> 7 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src & 0x03E0 ) >> 2 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src & 0x001F ) << 3 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R8( p_pSrc, p_val );
			G8( p_pSrc, p_val );
			B8( p_pSrc, p_val );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( src & 0x7FFF ) | ( p_val ? 0x8000 : 0x0000 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( src & 0x83FF ) | ( ( uint16_t( p_val ) << 7 ) & 0x7C00 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( src & 0xFC1F ) | ( ( uint16_t( p_val ) << 2 ) & 0x03E0 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( src & 0xFFE0 ) | ( ( uint16_t( p_val ) >> 3 ) & 0x001F );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			A8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eA4R4G4B4
	//!\~french		Spécialisation pour PixelFormat::eA4R4G4B4
	template<>
	struct component< PixelFormat::eA4R4G4B4 >
	{
#define src ( *reinterpret_cast< uint16_t const * >( p_pSrc ) )
#define dst ( *reinterpret_cast< uint16_t * >( p_pSrc ) )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( R8( p_pSrc ) * 0.30 + G8( p_pSrc ) * 0.59 + B8( p_pSrc ) * 0.11 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src >> 8 ) & 0x00F0 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src >> 4 ) & 0x00F0 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src >> 0 ) & 0x00F0 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src << 4 ) & 0x00F0 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R8( p_pSrc, p_val );
			G8( p_pSrc, p_val );
			B8( p_pSrc, p_val );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( dst & 0x0FFF ) | ( ( uint16_t( p_val ) << 8 ) & 0xF000 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( dst & 0xF0FF ) | ( ( uint16_t( p_val ) << 4 ) & 0x0F00 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( dst & 0xFF0F ) | ( ( uint16_t( p_val ) << 0 ) & 0x00F0 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( dst & 0xFFF0 ) | ( ( uint16_t( p_val ) >> 4 ) & 0x000F );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			A8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eR5G6B5
	//!\~french		Spécialisation pour PixelFormat::eR5G6B5
	template<>
	struct component< PixelFormat::eR5G6B5 >
	{
#define src	( *reinterpret_cast< uint16_t const * >( p_pSrc ) )
#define dst	( *reinterpret_cast< uint16_t * >( p_pSrc ) )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( R8( p_pSrc ) * 0.30 + G8( p_pSrc ) * 0.59 + B8( p_pSrc ) * 0.11 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src & 0xF800 ) >> 8 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src & 0x07E0 ) >> 3 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src & 0x001F ) << 3 );
		}
		static inline uint8_t A8( uint8_t const * )
		{
			return 0xFF;
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R8( p_pSrc, p_val );
			G8( p_pSrc, p_val );
			B8( p_pSrc, p_val );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( src & 0x07FF ) | ( ( uint16_t( p_val ) << 8 ) & 0xF800 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( src & 0xF81F ) | ( ( uint16_t( p_val ) << 3 ) & 0x07E0 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( src & 0xFFE0 ) | ( ( uint16_t( p_val ) >> 3 ) & 0x001F );
		}
		static inline void A8( uint8_t *, uint8_t )
		{
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t *, float )
		{
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eR8G8B8
	//!\~french		Spécialisation pour PixelFormat::eR8G8B8
	template<>
	struct component< PixelFormat::eR8G8B8 >
	{
		struct pixel
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
		};

#define src	( *reinterpret_cast< pixel const * >( p_pSrc ) )
#define dst	( *reinterpret_cast< pixel * >( p_pSrc ) )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( R8( p_pSrc ) * 0.30 + G8( p_pSrc ) * 0.59 + B8( p_pSrc ) * 0.11 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return src.r;
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return src.g;
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return src.b;
		}
		static inline uint8_t A8( uint8_t const * )
		{
			return 0xFF;
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R8( p_pSrc, p_val );
			G8( p_pSrc, p_val );
			B8( p_pSrc, p_val );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.r = p_val;
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.g = p_val;
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.b = p_val;
		}
		static inline void A8( uint8_t *, uint8_t )
		{
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t *, float )
		{
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eB8G8R8
	//!\~french		Spécialisation pour PixelFormat::eB8G8R8
	template<>
	struct component< PixelFormat::eB8G8R8 >
	{
		struct pixel
		{
			uint8_t b;
			uint8_t g;
			uint8_t r;
		};

#define src	( *reinterpret_cast< pixel const * >( p_pSrc ) )
#define dst	( *reinterpret_cast< pixel * >( p_pSrc ) )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( R8( p_pSrc ) * 0.30 + G8( p_pSrc ) * 0.59 + B8( p_pSrc ) * 0.11 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return src.r;
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return src.g;
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return src.b;
		}
		static inline uint8_t A8( uint8_t const * )
		{
			return 0xFF;
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R8( p_pSrc, p_val );
			G8( p_pSrc, p_val );
			B8( p_pSrc, p_val );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.r = p_val;
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.g = p_val;
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.b = p_val;
		}
		static inline void A8( uint8_t *, uint8_t )
		{
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t *, float )
		{
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eA8R8G8B8
	//!\~french		Spécialisation pour PixelFormat::eA8R8G8B8
	template<>
	struct component< PixelFormat::eA8R8G8B8 >
	{
		struct pixel
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};

#define src	( *reinterpret_cast< pixel const * >( p_pSrc ) )
#define dst	( *reinterpret_cast< pixel * >( p_pSrc ) )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( R8( p_pSrc ) * 0.30 + G8( p_pSrc ) * 0.59 + B8( p_pSrc ) * 0.11 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return src.a;
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return src.r;
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return src.g;
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return src.b;
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R8( p_pSrc, p_val );
			G8( p_pSrc, p_val );
			B8( p_pSrc, p_val );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.a = p_val;
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.r = p_val;
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.g = p_val;
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.b = p_val;
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			A8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eA8B8G8R8
	//!\~french		Spécialisation pour PixelFormat::eA8B8G8R8
	template<>
	struct component< PixelFormat::eA8B8G8R8 >
	{
		struct pixel
		{
			uint8_t b;
			uint8_t g;
			uint8_t r;
			uint8_t a;
		};

#define src	( *reinterpret_cast< pixel const * >( p_pSrc ) )
#define dst	( *reinterpret_cast< pixel * >( p_pSrc ) )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( R8( p_pSrc ) * 0.30 + G8( p_pSrc ) * 0.59 + B8( p_pSrc ) * 0.11 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return src.a;
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return src.r;
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return src.g;
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return src.b;
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R8( p_pSrc, p_val );
			G8( p_pSrc, p_val );
			B8( p_pSrc, p_val );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.a = p_val;
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.r = p_val;
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.g = p_val;
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst.b = p_val;
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			A8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eRGB16F
	//!\~french		Spécialisation pour PixelFormat::eRGB16F
	template<>
	struct component< PixelFormat::eRGB16F >
	{
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( R32F( p_pSrc ) * 255 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( G32F( p_pSrc ) * 255 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( B32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( A32F( p_pSrc ) * 255 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return float( R32F( p_pSrc ) * 0.30 + G32F( p_pSrc ) * 0.59 + B32F( p_pSrc ) * 0.11 );
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			float l_result{};
			HalfToFloat( l_result, reinterpret_cast< uint16_t const * >( p_pSrc ) + 0 );
			return l_result;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			float l_result{};
			HalfToFloat( l_result, reinterpret_cast< uint16_t const * >( p_pSrc ) + 1 );
			return l_result;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			float l_result{};
			HalfToFloat( l_result, reinterpret_cast< uint16_t const * >( p_pSrc ) + 2 );
			return l_result;
		}
		static inline float A32F( uint8_t const * )
		{
			return 1.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			G32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			B32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void A8( uint8_t *, uint8_t )
		{
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			R32F( p_pSrc, p_val );
			G32F( p_pSrc, p_val );
			B32F( p_pSrc, p_val );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			FloatToHalf( reinterpret_cast< uint16_t * >( p_pSrc ) + 0, p_val );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			FloatToHalf( reinterpret_cast< uint16_t * >( p_pSrc ) + 1, p_val );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			FloatToHalf( reinterpret_cast< uint16_t * >( p_pSrc ) + 2, p_val );
		}
		static inline void A32F( uint8_t *, float )
		{
		}
	};
	//!\~english	Specialisation for PixelFormat::eRGB16F32F
	//!\~french		Spécialisation pour PixelFormat::eRGB16F32F
	template<>
	struct component< PixelFormat::eRGB16F32F >
	{
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( R32F( p_pSrc ) * 255 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( G32F( p_pSrc ) * 255 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( B32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( A32F( p_pSrc ) * 255 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return float( R32F( p_pSrc ) * 0.30 + G32F( p_pSrc ) * 0.59 + B32F( p_pSrc ) * 0.11 );
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return reinterpret_cast< float const * >( p_pSrc )[0];
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return reinterpret_cast< float const * >( p_pSrc )[1];
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return reinterpret_cast< float const * >( p_pSrc )[2];
		}
		static inline float A32F( uint8_t const * )
		{
			return 1.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			G32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			B32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void A8( uint8_t *, uint8_t )
		{
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			R32F( p_pSrc, p_val );
			G32F( p_pSrc, p_val );
			B32F( p_pSrc, p_val );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			reinterpret_cast< float * >( p_pSrc )[0] = p_val;
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			reinterpret_cast< float * >( p_pSrc )[1] = p_val;
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			reinterpret_cast< float * >( p_pSrc )[2] = p_val;
		}
		static inline void A32F( uint8_t *, float )
		{
		}
	};
	//!\~english	Specialisation for PixelFormat::eRGB32F
	//!\~french		Spécialisation pour PixelFormat::eRGB32F
	template<>
	struct component< PixelFormat::eRGB32F >
	{
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( R32F( p_pSrc ) * 255 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( G32F( p_pSrc ) * 255 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( B32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( A32F( p_pSrc ) * 255 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return float( R32F( p_pSrc ) * 0.30 + G32F( p_pSrc ) * 0.59 + B32F( p_pSrc ) * 0.11 );
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return reinterpret_cast< float const * >( p_pSrc )[0];
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return reinterpret_cast< float const * >( p_pSrc )[1];
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return reinterpret_cast< float const * >( p_pSrc )[2];
		}
		static inline float A32F( uint8_t const * )
		{
			return 1.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			G32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			B32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void A8( uint8_t *, uint8_t )
		{
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			R32F( p_pSrc, p_val );
			G32F( p_pSrc, p_val );
			B32F( p_pSrc, p_val );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			reinterpret_cast< float * >( p_pSrc )[0] = p_val;
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			reinterpret_cast< float * >( p_pSrc )[1] = p_val;
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			reinterpret_cast< float * >( p_pSrc )[2] = p_val;
		}
		static inline void A32F( uint8_t *, float )
		{
		}
	};
	//!\~english	Specialisation for PixelFormat::eRGBA16F
	//!\~french		Spécialisation pour PixelFormat::eRGBA16F
	template<>
	struct component< PixelFormat::eRGBA16F >
	{
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( R32F( p_pSrc ) * 255 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( G32F( p_pSrc ) * 255 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( B32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( A32F( p_pSrc ) * 255 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return float( R32F( p_pSrc ) * 0.30 + G32F( p_pSrc ) * 0.59 + B32F( p_pSrc ) * 0.11 );
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			float l_result{};
			HalfToFloat( l_result, reinterpret_cast< uint16_t const * >( p_pSrc ) + 0 );
			return l_result;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			float l_result{};
			HalfToFloat( l_result, reinterpret_cast< uint16_t const * >( p_pSrc ) + 1 );
			return l_result;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			float l_result{};
			HalfToFloat( l_result, reinterpret_cast< uint16_t const * >( p_pSrc ) + 2 );
			return l_result;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			float l_result{};
			HalfToFloat( l_result, reinterpret_cast< uint16_t const * >( p_pSrc ) + 3 );
			return l_result;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			G32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			B32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			A32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			R32F( p_pSrc, p_val );
			G32F( p_pSrc, p_val );
			B32F( p_pSrc, p_val );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			FloatToHalf( reinterpret_cast< uint16_t * >( p_pSrc ) + 0, p_val );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			FloatToHalf( reinterpret_cast< uint16_t * >( p_pSrc ) + 1, p_val );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			FloatToHalf( reinterpret_cast< uint16_t * >( p_pSrc ) + 2, p_val );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			FloatToHalf( reinterpret_cast< uint16_t * >( p_pSrc ) + 3, p_val );
		}
	};
	//!\~english	Specialisation for PixelFormat::eRGBA16F32F
	//!\~french		Spécialisation pour PixelFormat::eRGBA16F32F
	template<>
	struct component< PixelFormat::eRGBA16F32F >
	{
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( R32F( p_pSrc ) * 255 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( G32F( p_pSrc ) * 255 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( B32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( A32F( p_pSrc ) * 255 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return float( R32F( p_pSrc ) * 0.30 + G32F( p_pSrc ) * 0.59 + B32F( p_pSrc ) * 0.11 );
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return reinterpret_cast< float const * >( p_pSrc )[0];
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return reinterpret_cast< float const * >( p_pSrc )[1];
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return reinterpret_cast< float const * >( p_pSrc )[2];
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return reinterpret_cast< float const * >( p_pSrc )[3];
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			G32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			B32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			A32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			R32F( p_pSrc, p_val );
			G32F( p_pSrc, p_val );
			B32F( p_pSrc, p_val );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			reinterpret_cast< float * >( p_pSrc )[0] = p_val;
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			reinterpret_cast< float * >( p_pSrc )[1] = p_val;
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			reinterpret_cast< float * >( p_pSrc )[2] = p_val;
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			reinterpret_cast< float * >( p_pSrc )[3] = p_val;
		}
	};
	//!\~english	Specialisation for PixelFormat::eRGBA32F
	//!\~french		Spécialisation pour PixelFormat::eRGBA32F
	template<>
	struct component< PixelFormat::eRGBA32F >
	{
#define src	( reinterpret_cast< float const * >( p_pSrc ) )
#define dst	( reinterpret_cast< float * >( p_pSrc ) )

		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( R32F( p_pSrc ) * 255 );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( G32F( p_pSrc ) * 255 );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( B32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return uint8_t( A32F( p_pSrc ) * 255 );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return float( R32F( p_pSrc ) * 0.30 + G32F( p_pSrc ) * 0.59 + B32F( p_pSrc ) * 0.11 );
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return src[0];
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return src[1];
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return src[2];
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return src[3];
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			R32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			G32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			B32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			A32F( p_pSrc, float( p_val ) / 255 );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			R32F( p_pSrc, p_val );
			G32F( p_pSrc, p_val );
			B32F( p_pSrc, p_val );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			dst[0] = p_val;
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			dst[1] = p_val;
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			dst[2] = p_val;
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			dst[3] = p_val;
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eYUY2
	//!\~french		Spécialisation pour PixelFormat::eYUY2
	template<>
	struct component< PixelFormat::eYUY2 >
	{
#	define YUV_Y(x)		(((x)[0] & 0xF0) & (((x)[1] & 0xF0) >> 2))
#	define YUV_U(x)		((x)[0] & 0x0F)
#	define YUV_V(x)		((x)[1] & 0x0F)
#	define YUV_C(x)		(YUV_Y(x) -  16)
#	define YUV_D(x)		(YUV_U(x) - 128)
#	define YUV_E(x)		(YUV_V(x) - 128)
#	define YUV_clip(x)	static_cast<uint8_t>( (x) > 255 ? 255 : (x) < 0 ? 0 : (x))
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( R8( p_pSrc ) * 0.30 + G8( p_pSrc ) * 0.59 + B8( p_pSrc ) * 0.11 );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return uint8_t( YUV_clip( ( 298 * YUV_C( p_pSrc ) + 409 * YUV_E( p_pSrc ) + 128 ) >> 8 ) );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return uint8_t( YUV_clip( ( 298 * YUV_C( p_pSrc ) - 100 * YUV_D( p_pSrc ) - 208 * YUV_E( p_pSrc ) + 128 ) >> 8 ) );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return uint8_t( YUV_clip( ( 298 * YUV_C( p_pSrc ) + 516 * YUV_D( p_pSrc ) + 128 ) >> 8 ) );
		}
		static inline uint8_t A8( uint8_t const * )
		{
			return 0xFF;
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
#	undef YUV_Y
#	undef YUV_U
#	undef YUV_V
#	undef YUV_C
#	undef YUV_D
#	undef YUV_E
#	undef YUV_clip
	};
	//!\~english	Specialisation for PixelFormat::eD16
	//!\~french		Spécialisation pour PixelFormat::eD16
	template<>
	struct component< PixelFormat::eD16 >
	{
#define src	( *reinterpret_cast< uint16_t const * >( p_pSrc ) )
#define dst	( *reinterpret_cast< uint16_t * >( p_pSrc ) )

		static inline uint8_t D8( uint8_t const * p_pSrc )
		{
			return uint8_t( src >> 8 );
		}
		static inline uint16_t D16( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline uint32_t D24( uint8_t const * p_pSrc )
		{
			return ( uint32_t( src ) << 8 );
		}
		static inline uint32_t D32( uint8_t const * p_pSrc )
		{
			return ( uint32_t( src ) << 16 );
		}
		static inline uint8_t S1( uint8_t const * )
		{
			return 0;
		}
		static inline uint8_t S8( uint8_t const * )
		{
			return 0;
		}
		static inline float D32F( uint8_t const * p_pSrc )
		{
			return float( D32( p_pSrc ) );
		}
		static inline float S32F( uint8_t const * )
		{
			return 0;
		}
		static inline void D8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( uint16_t( p_val ) << 8 );
		}
		static inline void D16( uint8_t * p_pSrc, uint16_t p_val )
		{
			dst = p_val;
		}
		static inline void D24( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = ( src & 0xFFFF0000 ) | ( 0x0000FFFF & ( p_val >> 8 ) );
		}
		static inline void D32( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = ( src & 0xFFFF0000 ) | ( 0x0000FFFF & ( p_val >> 16 ) );
		}
		static inline void S1( uint8_t *, uint8_t )
		{
		}
		static inline void S8( uint8_t *, uint8_t )
		{
		}
		static inline void D32F( uint8_t * p_pSrc, float p_val )
		{
			D32( p_pSrc, uint32_t( p_val ) );
		}
		static inline void S32F( uint8_t *, float )
		{
		}
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return D8( p_pSrc );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return 0;
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			D8( p_pSrc, p_val );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			A8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eD24
	//!\~french		Spécialisation pour PixelFormat::eD24
	template<>
	struct component< PixelFormat::eD24 >
	{
#define src	( *reinterpret_cast< uint32_t const * >( p_pSrc ) )
#define dst	( *reinterpret_cast< uint32_t * >( p_pSrc ) )

		static inline uint8_t D8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src & 0x00FFFFFF ) >> 16 );
		}
		static inline uint16_t D16( uint8_t const * p_pSrc )
		{
			return uint16_t( ( src & 0x00FFFFFF ) >> 8 );
		}
		static inline uint32_t D24( uint8_t const * p_pSrc )
		{
			return uint32_t( src & 0x00FFFFFF );
		}
		static inline uint32_t D32( uint8_t const * p_pSrc )
		{
			return ( uint32_t( src & 0x00FFFFFF ) << 8 );
		}
		static inline uint8_t S8( uint8_t const * )
		{
			return 0;
		}
		static inline float D32F( uint8_t const * p_pSrc )
		{
			return float( D32( p_pSrc ) );
		}
		static inline float S32F( uint8_t const * )
		{
			return 0;
		}
		static inline void D8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( src & 0xFF000000 ) | ( ( uint32_t( p_val ) << 16 ) & 0x00FFFFFF );
		}
		static inline void D16( uint8_t * p_pSrc, uint16_t p_val )
		{
			dst = ( src & 0xFF000000 ) | ( ( uint32_t( p_val ) << 8 ) & 0x00FFFFFF );
		}
		static inline void D24( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = ( src & 0xFF000000 ) | ( p_val & 0x00FFFFFF );
		}
		static inline void D32( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = ( src & 0xFF000000 ) | ( ( p_val >> 8 ) & 0x00FFFFFF );
		}
		static inline void S8( uint8_t *, uint8_t )
		{
		}
		static inline void D32F( uint8_t * p_pSrc, float p_val )
		{
			D32( p_pSrc, uint32_t( p_val ) );
		}
		static inline void S32F( uint8_t *, float )
		{
		}
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return D8( p_pSrc );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return 0;
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			D8( p_pSrc, p_val );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			A8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eD24S8
	//!\~french		Spécialisation pour PixelFormat::eD24S8
	template<>
	struct component< PixelFormat::eD24S8 >
	{
#define src	( *reinterpret_cast< uint32_t const * >( p_pSrc ) )
#define dst	( *reinterpret_cast< uint32_t * >( p_pSrc ) )

		static inline uint8_t	BIT( uint8_t const  p_bySrc )
		{
			return ( ( p_bySrc >> 0 ) & 0x01 ) | ( ( p_bySrc >> 1 ) & 0x01 ) | ( ( p_bySrc >> 2 ) & 0x01 ) | ( ( p_bySrc >> 3 ) & 0x01 ) | ( ( p_bySrc >> 4 ) & 0x01 ) | ( ( p_bySrc >> 5 ) & 0x01 ) | ( ( p_bySrc >> 6 ) & 0x01 ) | ( ( p_bySrc >> 7 ) & 0x01 );
		}
		static inline uint8_t D8( uint8_t const * p_pSrc )
		{
			return uint8_t( ( src & 0xFF000000 ) >> 24 );
		}
		static inline uint16_t D16( uint8_t const * p_pSrc )
		{
			return uint16_t( ( src & 0xFFFF0000 ) >> 16 );
		}
		static inline uint32_t D24( uint8_t const * p_pSrc )
		{
			return uint32_t( ( src & 0xFFFFFF00 ) >> 8 );
		}
		static inline uint32_t D32( uint8_t const * p_pSrc )
		{
			return uint32_t( ( src & 0xFFFFFF00 ) >> 8 );
		}
		static inline uint8_t	S1( uint8_t const * p_pSrc )
		{
			return ( BIT( p_pSrc[3] ) << 0 ) | ( BIT( p_pSrc[7] ) << 1 ) | ( BIT( p_pSrc[11] ) << 2 ) | ( BIT( p_pSrc[15] ) << 3 ) | ( BIT( p_pSrc[19] ) << 4 ) | ( BIT( p_pSrc[23] ) << 5 ) | ( BIT( p_pSrc[27] ) << 6 ) | ( BIT( p_pSrc[31] ) << 7 );
		}
		static inline uint8_t S8( uint8_t const * p_pSrc )
		{
			return uint8_t( src & 0x000000FF );
		}
		static inline float D32F( uint8_t const * p_pSrc )
		{
			return float( D32( p_pSrc ) );
		}
		static inline float S32F( uint8_t const * p_pSrc )
		{
			return float( S8( p_pSrc ) ) / 255.0f;
		}
		static inline void D8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( src & 0x000000FF ) | ( ( uint32_t( p_val ) << 24 ) & 0xFF000000 );
		}
		static inline void D16( uint8_t * p_pSrc, uint16_t p_val )
		{
			dst = ( src & 0x000000FF ) | ( ( uint32_t( p_val ) << 16 ) & 0xFFFF0000 );
		}
		static inline void D24( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = ( src & 0x000000FF ) | ( ( p_val << 8 ) & 0xFFFFFF00 );
		}
		static inline void D32( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = ( src & 0x000000FF ) | ( p_val & 0xFFFFFF00 );
		}
		static inline void S1( uint8_t * p_pSrc, uint8_t p_val )
		{
		}
		static inline void S8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( src & 0xFFFFFF00 ) | ( p_val & 0x000000FF );
		}
		static inline void D32F( uint8_t * p_pSrc, float p_val )
		{
			D32( p_pSrc, uint32_t( p_val ) );
		}
		static inline void S32F( uint8_t * p_pSrc, float p_val )
		{
			S8( p_pSrc, uint8_t( p_val * 255.0f ) );
		}
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return D8( p_pSrc );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return S8( p_pSrc );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			D8( p_pSrc, p_val );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			S8( p_pSrc, p_val );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			A8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eD32
	//!\~french		Spécialisation pour PixelFormat::eD32
	template<>
	struct component< PixelFormat::eD32 >
	{
#define src	( *reinterpret_cast< uint32_t const * >( p_pSrc ) )
#define dst	( *reinterpret_cast< uint32_t * >( p_pSrc ) )

		static inline uint8_t D8( uint8_t const * p_pSrc )
		{
			return uint8_t( src >> 24 );
		}
		static inline uint16_t D16( uint8_t const * p_pSrc )
		{
			return uint16_t( src >> 16 );
		}
		static inline uint32_t D24( uint8_t const * p_pSrc )
		{
			return ( src >> 8 );
		}
		static inline uint32_t D32( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline uint8_t S1( uint8_t const * )
		{
			return 0;
		}
		static inline uint8_t S8( uint8_t const * )
		{
			return 0;
		}
		static inline float D32F( uint8_t const * p_pSrc )
		{
			return float( D32( p_pSrc ) );
		}
		static inline float S32F( uint8_t const * )
		{
			return 0;
		}
		static inline void D8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = ( uint32_t( p_val ) << 24 );
		}
		static inline void D16( uint8_t * p_pSrc, uint16_t p_val )
		{
			dst = ( uint32_t( p_val ) << 16 );
		}
		static inline void D24( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = ( p_val << 8 );
		}
		static inline void D32( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = p_val;
		}
		static inline void S1( uint8_t *, uint8_t )
		{
		}
		static inline void S8( uint8_t *, uint8_t )
		{
		}
		static inline void D32F( uint8_t * p_pSrc, float p_val )
		{
			D32( p_pSrc, uint32_t( p_val ) );
		}
		static inline void S32F( uint8_t *, float )
		{
		}
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return D8( p_pSrc );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return 0xFF;
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			D8( p_pSrc, p_val );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			A8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eD32F
	//!\~french		Spécialisation pour PixelFormat::eD32F
	template<>
	struct component< PixelFormat::eD32F >
	{
#define src	( *reinterpret_cast< float const * >( p_pSrc ) )
#define dst	( *reinterpret_cast< float * >( p_pSrc ) )

		static inline float D32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float S32F( uint8_t const * )
		{
			return 0;
		}
		static inline uint8_t D8( uint8_t const * p_pSrc )
		{
			return uint8_t( src * 256 );
		}
		static inline uint16_t D16( uint8_t const * p_pSrc )
		{
			return uint8_t( src * 65536 );
		}
		static inline uint32_t D24( uint8_t const * p_pSrc )
		{
			return uint32_t( src * 65536 * 256 );
		}
		static inline uint32_t D32( uint8_t const * p_pSrc )
		{
			return uint32_t( src * 65536 * 65536 );
		}
		static inline uint8_t S1( uint8_t const * )
		{
			return 0;
		}
		static inline uint8_t S8( uint8_t const * )
		{
			return 0;
		}
		static inline void D32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void S32F( uint8_t *, float )
		{
		}
		static inline void D8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = float( p_val ) / float( std::numeric_limits< uint8_t >::max() );
		}
		static inline void D16( uint8_t * p_pSrc, uint16_t p_val )
		{
			dst = float( p_val ) / float( std::numeric_limits< uint16_t >::max() );
		}
		static inline void D24( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = float( p_val ) / float( std::numeric_limits< uint16_t >::max() * std::numeric_limits< uint8_t >::max() );
		}
		static inline void D32( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = float( p_val ) / float( std::numeric_limits< uint32_t >::max() );
		}
		static inline void S1( uint8_t *, uint8_t )
		{
		}
		static inline void S8( uint8_t *, uint8_t )
		{
		}
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return 0xFF;
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return L32F( p_pSrc );
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return L32F( p_pSrc );
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return L32F( p_pSrc );
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return 1.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			D8( p_pSrc, p_val );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			L32F( p_pSrc, p_val );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			L32F( p_pSrc, p_val );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			L32F( p_pSrc, p_val );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			L32F( p_pSrc, p_val );
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eD32FS8
	//!\~french		Spécialisation pour PixelFormat::eD32FS8
	template<>
	struct component< PixelFormat::eD32FS8 >
	{
#define src	( *reinterpret_cast< float const * >( p_pSrc ) )
#define dst	( *reinterpret_cast< float * >( p_pSrc ) )

		static inline float D32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float S32F( uint8_t const * )
		{
			return 0;
		}
		static inline uint8_t D8( uint8_t const * p_pSrc )
		{
			return uint8_t( src * 256 );
		}
		static inline uint16_t D16( uint8_t const * p_pSrc )
		{
			return uint8_t( src * 65536 );
		}
		static inline uint32_t D24( uint8_t const * p_pSrc )
		{
			return uint32_t( src * 65536 * 256 );
		}
		static inline uint32_t D32( uint8_t const * p_pSrc )
		{
			return uint32_t( src * 65536 * 65536 );
		}
		static inline uint8_t S1( uint8_t const * )
		{
			return 0;
		}
		static inline uint8_t S8( uint8_t const * )
		{
			return 0;
		}
		static inline void D32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void S32F( uint8_t *, float )
		{
		}
		static inline void D8( uint8_t * p_pSrc, uint8_t p_val )
		{
			dst = float( p_val ) / float( std::numeric_limits< uint8_t >::max() );
		}
		static inline void D16( uint8_t * p_pSrc, uint16_t p_val )
		{
			dst = float( p_val ) / float( std::numeric_limits< uint16_t >::max() );
		}
		static inline void D24( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = float( p_val ) / float( std::numeric_limits< uint16_t >::max() * std::numeric_limits< uint8_t >::max() );
		}
		static inline void D32( uint8_t * p_pSrc, uint32_t p_val )
		{
			dst = float( p_val ) / float( std::numeric_limits< uint32_t >::max() );
		}
		static inline void S1( uint8_t *, uint8_t )
		{
		}
		static inline void S8( uint8_t *, uint8_t )
		{
		}
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return uint8_t( L32F( p_pSrc ) * 255 );
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return 0xFF;
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return src;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return L32F( p_pSrc );
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return L32F( p_pSrc );
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return L32F( p_pSrc );
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return 1.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
			D8( p_pSrc, p_val );
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			dst = p_val;
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			L32F( p_pSrc, p_val );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			L32F( p_pSrc, p_val );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			L32F( p_pSrc, p_val );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			L32F( p_pSrc, p_val );
		}

#undef src
#undef dst
	};
	//!\~english	Specialisation for PixelFormat::eS8
	//!\~french		Spécialisation pour PixelFormat::eS8
	template<>
	struct component< PixelFormat::eS8 >
	{
		static inline uint8_t	BIT( uint8_t const  p_bySrc )
		{
			return ( ( p_bySrc >> 0 ) & 0x01 ) | ( ( p_bySrc >> 1 ) & 0x01 ) | ( ( p_bySrc >> 2 ) & 0x01 ) | ( ( p_bySrc >> 3 ) & 0x01 ) | ( ( p_bySrc >> 4 ) & 0x01 ) | ( ( p_bySrc >> 5 ) & 0x01 ) | ( ( p_bySrc >> 6 ) & 0x01 ) | ( ( p_bySrc >> 7 ) & 0x01 );
		}
		static inline uint16_t D16( uint8_t const * )
		{
			return 0;
		}
		static inline uint32_t D24( uint8_t const * )
		{
			return 0;
		}
		static inline uint32_t D32( uint8_t const * )
		{
			return 0;
		}
		static inline uint8_t S1( uint8_t const * p_pSrc )
		{
			return ( BIT( p_pSrc[3] ) << 0 ) | ( BIT( p_pSrc[7] ) << 1 ) | ( BIT( p_pSrc[11] ) << 2 ) | ( BIT( p_pSrc[15] ) << 3 ) | ( BIT( p_pSrc[19] ) << 4 ) | ( BIT( p_pSrc[23] ) << 5 ) | ( BIT( p_pSrc[27] ) << 6 ) | ( BIT( p_pSrc[31] ) << 7 );
		}
		static inline uint8_t S8( uint8_t const * p_pSrc )
		{
			return *p_pSrc;
		}
		static inline float D32F( uint8_t const * )
		{
			return 0;
		}
		static inline float S32F( uint8_t const * p_pSrc )
		{
			return float( S8( p_pSrc ) ) / 255.0f;
		}
		static inline void D16( uint8_t *, uint16_t )
		{
		}
		static inline void D24( uint8_t *, uint32_t )
		{
		}
		static inline void D32( uint8_t *, uint32_t )
		{
		}
		static inline void S1( uint8_t * p_pSrc, uint8_t p_val )
		{
		}
		static inline void S8( uint8_t * p_pSrc, uint8_t p_val )
		{
			*p_pSrc = p_val;
		}
		static inline void D32F( uint8_t *, float )
		{
		}
		static inline void S32F( uint8_t * p_pSrc, float p_val )
		{
			S8( p_pSrc, uint8_t( p_val * 255.0f ) );
		}
		static inline uint8_t L8( uint8_t const * p_pSrc )
		{
			return 0;
		}
		static inline uint8_t A8( uint8_t const * p_pSrc )
		{
			return S8( p_pSrc );
		}
		static inline uint8_t R8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t G8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline uint8_t B8( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc );
		}
		static inline float L32F( uint8_t const * p_pSrc )
		{
			return L8( p_pSrc ) / 255.0f;
		}
		static inline float R32F( uint8_t const * p_pSrc )
		{
			return R8( p_pSrc ) / 255.0f;
		}
		static inline float G32F( uint8_t const * p_pSrc )
		{
			return G8( p_pSrc ) / 255.0f;
		}
		static inline float B32F( uint8_t const * p_pSrc )
		{
			return B8( p_pSrc ) / 255.0f;
		}
		static inline float A32F( uint8_t const * p_pSrc )
		{
			return A8( p_pSrc ) / 255.0f;
		}
		static inline void L8( uint8_t * p_pSrc, uint8_t p_val )
		{
		}
		static inline void A8( uint8_t * p_pSrc, uint8_t p_val )
		{
			S8( p_pSrc, p_val );
		}
		static inline void R8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void G8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void B8( uint8_t * p_pSrc, uint8_t p_val )
		{
			L8( p_pSrc, p_val );
		}
		static inline void L32F( uint8_t * p_pSrc, float p_val )
		{
			L8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void R32F( uint8_t * p_pSrc, float p_val )
		{
			R8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void G32F( uint8_t * p_pSrc, float p_val )
		{
			G8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void B32F( uint8_t * p_pSrc, float p_val )
		{
			B8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
		static inline void A32F( uint8_t * p_pSrc, float p_val )
		{
			A8( p_pSrc, uint8_t( p_val * 255.0 ) );
		}
	};
	namespace detail
	{
		/*!
		\author 	Sylvain DOREMUS
		\date 		27/05/2013
		\~english
		\brief		Structure used to convert one pixel from one format to another one
		\~french
		\brief		Structure utilisée pour convertir un pixel d'un format vers un autre
		*/
		template< PixelFormat PFSrc, PixelFormat PFDst, typename Enable = void > struct pixel_converter;
		//!\~english	Specialisation when source and destination formats are the same
		//!\~french		Spécialisation quand les formats source et destination sont identiques
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter< PFSrc, PFDst, typename std::enable_if< PFSrc == PFDst >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				std::memcpy( p_pDst, p_pSrc, pixel_definitions< PFSrc >::Size );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eL8
		//!\~french		Spécialisation pour convertir vers PixelFormat::eL8
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eL8 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::L8( p_pDst, component< PFSrc >::L8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eL16F32F
		//!\~french		Spécialisation pour convertir vers PixelFormat::eL16F32F
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eL16F32F >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::L32F( p_pDst, component< PFSrc >::L32F( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eL32F
		//!\~french		Spécialisation pour convertir vers PixelFormat::eL32F
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eL32F >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::L32F( p_pDst, component< PFSrc >::L32F( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eA8L8
		//!\~french		Spécialisation pour convertir vers PixelFormat::eA8L8
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eA8L8 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::L8( p_pDst, component< PFSrc >::L8( p_pSrc ) );
				component< PFDst >::A8( p_pDst, component< PFSrc >::A8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eAL16F32F
		//!\~french		Spécialisation pour convertir vers PixelFormat::eAL16F32F
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eAL16F32F >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::L32F( p_pDst, component< PFSrc >::L32F( p_pSrc ) );
				component< PFDst >::A32F( p_pDst, component< PFSrc >::A32F( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eAL32F
		//!\~french		Spécialisation pour convertir vers PixelFormat::eAL32F
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eAL32F >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::L32F( p_pDst, component< PFSrc >::L32F( p_pSrc ) );
				component< PFDst >::A32F( p_pDst, component< PFSrc >::A32F( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eA1R5G5B5
		//!\~french		Spécialisation pour convertir vers PixelFormat::eA1R5G5B5
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eA1R5G5B5 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::A8( p_pDst, component< PFSrc >::A8( p_pSrc ) );
				component< PFDst >::R8( p_pDst, component< PFSrc >::R8( p_pSrc ) );
				component< PFDst >::G8( p_pDst, component< PFSrc >::G8( p_pSrc ) );
				component< PFDst >::B8( p_pDst, component< PFSrc >::B8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eA4R4G4B4
		//!\~french		Spécialisation pour convertir vers PixelFormat::eA4R4G4B4
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eA4R4G4B4 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::A8( p_pDst, component< PFSrc >::A8( p_pSrc ) );
				component< PFDst >::R8( p_pDst, component< PFSrc >::R8( p_pSrc ) );
				component< PFDst >::G8( p_pDst, component< PFSrc >::G8( p_pSrc ) );
				component< PFDst >::B8( p_pDst, component< PFSrc >::B8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eR5G6B5
		//!\~french		Spécialisation pour convertir vers PixelFormat::eR5G6B5
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eR5G6B5 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::R8( p_pDst, component< PFSrc >::R8( p_pSrc ) );
				component< PFDst >::G8( p_pDst, component< PFSrc >::G8( p_pSrc ) );
				component< PFDst >::B8( p_pDst, component< PFSrc >::B8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eR8G8B8
		//!\~french		Spécialisation pour convertir vers PixelFormat::eR8G8B8
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eR8G8B8 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::R8( p_pDst, component< PFSrc >::R8( p_pSrc ) );
				component< PFDst >::G8( p_pDst, component< PFSrc >::G8( p_pSrc ) );
				component< PFDst >::B8( p_pDst, component< PFSrc >::B8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eB8G8R8
		//!\~french		Spécialisation pour convertir vers PixelFormat::eB8G8R8
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eB8G8R8 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::B8( p_pDst, component< PFSrc >::B8( p_pSrc ) );
				component< PFDst >::G8( p_pDst, component< PFSrc >::G8( p_pSrc ) );
				component< PFDst >::R8( p_pDst, component< PFSrc >::R8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eA8R8G8B8
		//!\~french		Spécialisation pour convertir vers PixelFormat::eA8R8G8B8
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eA8R8G8B8 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::A8( p_pDst, component< PFSrc >::A8( p_pSrc ) );
				component< PFDst >::R8( p_pDst, component< PFSrc >::R8( p_pSrc ) );
				component< PFDst >::G8( p_pDst, component< PFSrc >::G8( p_pSrc ) );
				component< PFDst >::B8( p_pDst, component< PFSrc >::B8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eA8R8G8B8
		//!\~french		Spécialisation pour convertir vers PixelFormat::eA8R8G8B8
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eA8B8G8R8 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::A8( p_pDst, component< PFSrc >::A8( p_pSrc ) );
				component< PFDst >::B8( p_pDst, component< PFSrc >::B8( p_pSrc ) );
				component< PFDst >::G8( p_pDst, component< PFSrc >::G8( p_pSrc ) );
				component< PFDst >::R8( p_pDst, component< PFSrc >::R8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eRGB16F
		//!\~french		Spécialisation pour convertir vers PixelFormat::eRGB16F
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eRGB16F >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::R32F( p_pDst, component< PFSrc >::R32F( p_pSrc ) );
				component< PFDst >::G32F( p_pDst, component< PFSrc >::G32F( p_pSrc ) );
				component< PFDst >::B32F( p_pDst, component< PFSrc >::B32F( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eRGB16F32F
		//!\~french		Spécialisation pour convertir vers PixelFormat::eRGB16F32F
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eRGB16F32F >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::R32F( p_pDst, component< PFSrc >::R32F( p_pSrc ) );
				component< PFDst >::G32F( p_pDst, component< PFSrc >::G32F( p_pSrc ) );
				component< PFDst >::B32F( p_pDst, component< PFSrc >::B32F( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eRGB32F
		//!\~french		Spécialisation pour convertir vers PixelFormat::eRGB32F
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eRGB32F >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::R32F( p_pDst, component< PFSrc >::R32F( p_pSrc ) );
				component< PFDst >::G32F( p_pDst, component< PFSrc >::G32F( p_pSrc ) );
				component< PFDst >::B32F( p_pDst, component< PFSrc >::B32F( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eRGBA16F
		//!\~french		Spécialisation pour convertir vers PixelFormat::eRGBA16F
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eRGBA16F >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::A32F( p_pDst, component< PFSrc >::A32F( p_pSrc ) );
				component< PFDst >::R32F( p_pDst, component< PFSrc >::R32F( p_pSrc ) );
				component< PFDst >::G32F( p_pDst, component< PFSrc >::G32F( p_pSrc ) );
				component< PFDst >::B32F( p_pDst, component< PFSrc >::B32F( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eRGBA16F32F
		//!\~french		Spécialisation pour convertir vers PixelFormat::eRGBA16F32F
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eRGBA16F32F >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::A32F( p_pDst, component< PFSrc >::A32F( p_pSrc ) );
				component< PFDst >::R32F( p_pDst, component< PFSrc >::R32F( p_pSrc ) );
				component< PFDst >::G32F( p_pDst, component< PFSrc >::G32F( p_pSrc ) );
				component< PFDst >::B32F( p_pDst, component< PFSrc >::B32F( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eRGBA32F
		//!\~french		Spécialisation pour convertir vers PixelFormat::eRGBA32F
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eRGBA32F >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::A32F( p_pDst, component< PFSrc >::A32F( p_pSrc ) );
				component< PFDst >::R32F( p_pDst, component< PFSrc >::R32F( p_pSrc ) );
				component< PFDst >::G32F( p_pDst, component< PFSrc >::G32F( p_pSrc ) );
				component< PFDst >::B32F( p_pDst, component< PFSrc >::B32F( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eYUY2
		//!\~french		Spécialisation pour convertir vers PixelFormat::eYUY2
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eYUY2 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				p_pDst[0] = ( ( ( ( ( 66 * component< PFSrc >::R8( p_pSrc ) + 129 * component< PFSrc >::G8( p_pSrc ) +  25 * component< PFSrc >::B8( p_pSrc ) + 128 ) >> 8 ) +  16 ) & 0x0F ) << 4 ) | ( ( ( ( -38 * component< PFSrc >::R8( p_pSrc ) - 74 * component< PFSrc >::G8( p_pSrc ) + 112 * component< PFSrc >::B8( p_pSrc ) + 128 ) >> 8 ) + 128 ) & 0x0F );
				p_pDst[1] = ( ( ( ( ( 66 * component< PFSrc >::R8( p_pSrc ) + 129 * component< PFSrc >::G8( p_pSrc ) +  25 * component< PFSrc >::B8( p_pSrc ) + 128 ) >> 8 ) +  16 ) & 0x0F ) << 4 ) | ( ( ( ( 112 * component< PFSrc >::R8( p_pSrc ) - 94 * component< PFSrc >::G8( p_pSrc ) -  18 * component< PFSrc >::B8( p_pSrc ) + 128 ) >> 8 ) + 128 ) & 0x0F );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eD16
		//!\~french		Spécialisation pour convertir vers PixelFormat::eD16
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eD16 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::D16( p_pDst, component< PFSrc >::D16( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eD24
		//!\~french		Spécialisation pour convertir vers PixelFormat::eD24
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eD24 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::D24( p_pDst, component< PFSrc >::D24( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eD24S8
		//!\~french		Spécialisation pour convertir vers PixelFormat::eD24S8
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFSrc != PixelFormat::eS1 && PFDst == PixelFormat::eD24S8 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::D24( p_pDst, component< PFSrc >::D24( p_pSrc ) );
				component< PFDst >::S8( p_pDst, component< PFSrc >::S8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eD32
		//!\~french		Spécialisation pour convertir vers PixelFormat::eD32
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eD32 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::D32( p_pDst, component< PFSrc >::D32( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eD32F
		//!\~french		Spécialisation pour convertir vers PixelFormat::eD32F
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eD32F >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::D32F( p_pDst, component< PFSrc >::D32F( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eD32FS8
		//!\~french		Spécialisation pour convertir vers PixelFormat::eD32FS8
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eD32FS8 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::D32F( p_pDst, component< PFSrc >::D32F( p_pSrc ) );
				component< PFDst >::S8( p_pDst, component< PFSrc >::S8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eS1
		//!\~french		Spécialisation pour convertir vers PixelFormat::eS1
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFDst == PixelFormat::eS1 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				*p_pDst = component< PFSrc >::S1( p_pSrc );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for converting to PixelFormat::eS8
		//!\~french		Spécialisation pour convertir vers PixelFormat::eS8
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFSrc != PixelFormat::eS1 && PFDst == PixelFormat::eS8 >::type >
		{
			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				component< PFDst >::S8( p_pDst, component< PFSrc >::S8( p_pSrc ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for convertions from PixelFormat::eS1 to PixelFormat::eS8
		//!\~french		Spécialisation pour convertir de PixelFormat::eS1 vers PixelFormat::eS8
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFSrc == PixelFormat::eS1 && PFDst == PixelFormat::eS8 >::type >
		{
			inline uint8_t BIT( uint8_t const src, uint8_t uiIndex )
			{
				return ( ( src >> uiIndex ) & 0x01 );
			}

			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				p_pDst[0] = ( BIT( *p_pSrc, 0 ) << 0 ) | ( BIT( *p_pSrc, 0 ) << 1 ) | ( BIT( *p_pSrc, 0 ) << 2 ) | ( BIT( *p_pSrc, 0 ) << 3 ) | ( BIT( *p_pSrc, 0 ) << 4 ) | ( BIT( *p_pSrc, 0 ) << 5 ) | ( BIT( *p_pSrc, 0 ) << 6 ) | ( BIT( *p_pSrc, 0 ) << 7 );
				p_pDst[1] = ( BIT( *p_pSrc, 1 ) << 0 ) | ( BIT( *p_pSrc, 1 ) << 1 ) | ( BIT( *p_pSrc, 1 ) << 2 ) | ( BIT( *p_pSrc, 1 ) << 3 ) | ( BIT( *p_pSrc, 1 ) << 4 ) | ( BIT( *p_pSrc, 1 ) << 5 ) | ( BIT( *p_pSrc, 1 ) << 6 ) | ( BIT( *p_pSrc, 1 ) << 7 );
				p_pDst[2] = ( BIT( *p_pSrc, 2 ) << 0 ) | ( BIT( *p_pSrc, 2 ) << 1 ) | ( BIT( *p_pSrc, 2 ) << 2 ) | ( BIT( *p_pSrc, 2 ) << 3 ) | ( BIT( *p_pSrc, 2 ) << 4 ) | ( BIT( *p_pSrc, 2 ) << 5 ) | ( BIT( *p_pSrc, 2 ) << 6 ) | ( BIT( *p_pSrc, 2 ) << 7 );
				p_pDst[3] = ( BIT( *p_pSrc, 3 ) << 0 ) | ( BIT( *p_pSrc, 3 ) << 1 ) | ( BIT( *p_pSrc, 3 ) << 2 ) | ( BIT( *p_pSrc, 3 ) << 3 ) | ( BIT( *p_pSrc, 3 ) << 4 ) | ( BIT( *p_pSrc, 3 ) << 5 ) | ( BIT( *p_pSrc, 3 ) << 6 ) | ( BIT( *p_pSrc, 3 ) << 7 );
				p_pDst[4] = ( BIT( *p_pSrc, 4 ) << 0 ) | ( BIT( *p_pSrc, 4 ) << 1 ) | ( BIT( *p_pSrc, 4 ) << 2 ) | ( BIT( *p_pSrc, 4 ) << 3 ) | ( BIT( *p_pSrc, 4 ) << 4 ) | ( BIT( *p_pSrc, 4 ) << 5 ) | ( BIT( *p_pSrc, 4 ) << 6 ) | ( BIT( *p_pSrc, 4 ) << 7 );
				p_pDst[5] = ( BIT( *p_pSrc, 5 ) << 0 ) | ( BIT( *p_pSrc, 5 ) << 1 ) | ( BIT( *p_pSrc, 5 ) << 2 ) | ( BIT( *p_pSrc, 5 ) << 3 ) | ( BIT( *p_pSrc, 5 ) << 4 ) | ( BIT( *p_pSrc, 5 ) << 5 ) | ( BIT( *p_pSrc, 5 ) << 6 ) | ( BIT( *p_pSrc, 5 ) << 7 );
				p_pDst[6] = ( BIT( *p_pSrc, 6 ) << 0 ) | ( BIT( *p_pSrc, 6 ) << 1 ) | ( BIT( *p_pSrc, 6 ) << 2 ) | ( BIT( *p_pSrc, 6 ) << 3 ) | ( BIT( *p_pSrc, 6 ) << 4 ) | ( BIT( *p_pSrc, 6 ) << 5 ) | ( BIT( *p_pSrc, 6 ) << 6 ) | ( BIT( *p_pSrc, 6 ) << 7 );
				p_pDst[7] = ( BIT( *p_pSrc, 7 ) << 0 ) | ( BIT( *p_pSrc, 7 ) << 1 ) | ( BIT( *p_pSrc, 7 ) << 2 ) | ( BIT( *p_pSrc, 7 ) << 3 ) | ( BIT( *p_pSrc, 7 ) << 4 ) | ( BIT( *p_pSrc, 7 ) << 5 ) | ( BIT( *p_pSrc, 7 ) << 6 ) | ( BIT( *p_pSrc, 7 ) << 7 );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		//!\~english	Specialisation for convertions from PixelFormat::eS1 to PixelFormat::eD24S8
		//!\~french		Spécialisation pour convertir de PixelFormat::eS1 vers PixelFormat::eD24S8
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct pixel_converter < PFSrc, PFDst, typename std::enable_if < PFSrc != PFDst && PFSrc == PixelFormat::eS1 && PFDst == PixelFormat::eD24S8 >::type >
		{
			inline uint8_t BIT( uint8_t const src, uint8_t uiIndex )
			{
				return ( ( src >> uiIndex ) & 0x01 );
			}

			inline void operator()( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
			{
				uint32_t * l_pDst = reinterpret_cast< uint32_t * >( &p_pDst[0] );
				l_pDst[0] = 0xFFFFFF00 + ( ( BIT( *p_pSrc, 0 ) << 0 ) | ( BIT( *p_pSrc, 0 ) << 1 ) | ( BIT( *p_pSrc, 0 ) << 2 ) | ( BIT( *p_pSrc, 0 ) << 3 ) | ( BIT( *p_pSrc, 0 ) << 4 ) | ( BIT( *p_pSrc, 0 ) << 5 ) | ( BIT( *p_pSrc, 0 ) << 6 ) | ( BIT( *p_pSrc, 0 ) << 7 ) );
				l_pDst[1] = 0xFFFFFF00 + ( ( BIT( *p_pSrc, 1 ) << 0 ) | ( BIT( *p_pSrc, 1 ) << 1 ) | ( BIT( *p_pSrc, 1 ) << 2 ) | ( BIT( *p_pSrc, 1 ) << 3 ) | ( BIT( *p_pSrc, 1 ) << 4 ) | ( BIT( *p_pSrc, 1 ) << 5 ) | ( BIT( *p_pSrc, 1 ) << 6 ) | ( BIT( *p_pSrc, 1 ) << 7 ) );
				l_pDst[2] = 0xFFFFFF00 + ( ( BIT( *p_pSrc, 2 ) << 0 ) | ( BIT( *p_pSrc, 2 ) << 1 ) | ( BIT( *p_pSrc, 2 ) << 2 ) | ( BIT( *p_pSrc, 2 ) << 3 ) | ( BIT( *p_pSrc, 2 ) << 4 ) | ( BIT( *p_pSrc, 2 ) << 5 ) | ( BIT( *p_pSrc, 2 ) << 6 ) | ( BIT( *p_pSrc, 2 ) << 7 ) );
				l_pDst[3] = 0xFFFFFF00 + ( ( BIT( *p_pSrc, 3 ) << 0 ) | ( BIT( *p_pSrc, 3 ) << 1 ) | ( BIT( *p_pSrc, 3 ) << 2 ) | ( BIT( *p_pSrc, 3 ) << 3 ) | ( BIT( *p_pSrc, 3 ) << 4 ) | ( BIT( *p_pSrc, 3 ) << 5 ) | ( BIT( *p_pSrc, 3 ) << 6 ) | ( BIT( *p_pSrc, 3 ) << 7 ) );
				l_pDst[4] = 0xFFFFFF00 + ( ( BIT( *p_pSrc, 4 ) << 0 ) | ( BIT( *p_pSrc, 4 ) << 1 ) | ( BIT( *p_pSrc, 4 ) << 2 ) | ( BIT( *p_pSrc, 4 ) << 3 ) | ( BIT( *p_pSrc, 4 ) << 4 ) | ( BIT( *p_pSrc, 4 ) << 5 ) | ( BIT( *p_pSrc, 4 ) << 6 ) | ( BIT( *p_pSrc, 4 ) << 7 ) );
				l_pDst[5] = 0xFFFFFF00 + ( ( BIT( *p_pSrc, 5 ) << 0 ) | ( BIT( *p_pSrc, 5 ) << 1 ) | ( BIT( *p_pSrc, 5 ) << 2 ) | ( BIT( *p_pSrc, 5 ) << 3 ) | ( BIT( *p_pSrc, 5 ) << 4 ) | ( BIT( *p_pSrc, 5 ) << 5 ) | ( BIT( *p_pSrc, 5 ) << 6 ) | ( BIT( *p_pSrc, 5 ) << 7 ) );
				l_pDst[6] = 0xFFFFFF00 + ( ( BIT( *p_pSrc, 6 ) << 0 ) | ( BIT( *p_pSrc, 6 ) << 1 ) | ( BIT( *p_pSrc, 6 ) << 2 ) | ( BIT( *p_pSrc, 6 ) << 3 ) | ( BIT( *p_pSrc, 6 ) << 4 ) | ( BIT( *p_pSrc, 6 ) << 5 ) | ( BIT( *p_pSrc, 6 ) << 6 ) | ( BIT( *p_pSrc, 6 ) << 7 ) );
				l_pDst[7] = 0xFFFFFF00 + ( ( BIT( *p_pSrc, 7 ) << 0 ) | ( BIT( *p_pSrc, 7 ) << 1 ) | ( BIT( *p_pSrc, 7 ) << 2 ) | ( BIT( *p_pSrc, 7 ) << 3 ) | ( BIT( *p_pSrc, 7 ) << 4 ) | ( BIT( *p_pSrc, 7 ) << 5 ) | ( BIT( *p_pSrc, 7 ) << 6 ) | ( BIT( *p_pSrc, 7 ) << 7 ) );
				p_pSrc += pixel_definitions< PFSrc >::Size;
				p_pDst += pixel_definitions< PFDst >::Size;
			}
		};
		/*!
		\author 	Sylvain DOREMUS
		\date 		08/09/2013
		\~english
		\brief		Structure used to convert a buffer from one pixel format to another one
		\~french
		\brief		Structure utilisée pour convertir un buffer d'un format de pixels vers un autre
		*/
		template< PixelFormat PFSrc, PixelFormat PFDst >
		struct buffer_converter
		{
			inline void operator()( uint8_t const * p_pSrcBuffer, uint32_t p_uiSrcSize, uint8_t * p_pDstBuffer, uint32_t p_uiDstSize )
			{
				uint8_t const * l_pSrc = &p_pSrcBuffer[0];
				uint8_t * l_pDst = &p_pDstBuffer[0];
				uint32_t l_uiSrcCount = 0;
				uint32_t l_count = p_uiSrcSize / pixel_definitions< PFSrc >::Size;
				REQUIRE( p_uiSrcSize / pixel_definitions< PFSrc >::Size == p_uiDstSize / pixel_definitions< PFDst >::Size );
				pixel_converter< PFSrc, PFDst > l_converter;

				for ( uint32_t i = 0; i < l_count; i++ )
				{
					l_converter( l_pSrc, l_pDst );
				}
			}
		};

#if defined( _X64 ) && CASTOR_USE_SSE2

		/*!
		\author 	Sylvain DOREMUS
		\date 		08/09/2013
		\~english
		\brief		Specialisation for YUY2 to A8R8G8B8 conversion
		\~french
		\brief		Spécialisation pour la conversion YUY2 vers A8R8G8B8
		*/
		template<>
		struct buffer_converter< PixelFormat::eYUY2, PixelFormat::eA8R8G8B8 >
		{
			static const __m128i amask;
			static const __m128i umask;
			static const __m128i vmask;
			static const __m128i ymask;
			static const __m128i uvsub;
			static const __m128i ysub;
			static const __m128i facy;
			static const __m128i facrv;
			static const __m128i facgu;
			static const __m128i facgv;
			static const __m128i facbu;
			static const __m128i zero;

			inline void operator()( uint8_t const * p_pSrcBuffer, uint32_t p_uiSrcSize, uint8_t * p_pDstBuffer, uint32_t p_uiDstSize )
			{
				__m128i yuv;
				__m128i y;
				__m128i	u;
				__m128i	v;
				__m128i rv0;
				__m128i gu0;
				__m128i gv0;
				__m128i bu0;
				__m128i r0;
				__m128i g0;
				__m128i b0;
				__m128i r1;
				__m128i gb;
				__m128i argb0123;
				__m128i argb4567;
				__m128i const *	l_pBufferIn		= l_pBufferIn = reinterpret_cast< __m128i const * >( p_pSrcBuffer );
				__m128i const *	l_pBufferInEnd	= reinterpret_cast< __m128i const * >( p_pSrcBuffer + p_uiSrcSize );
				__m128i const *	l_pLineIn		= l_pBufferIn;
				__m128i 	*	l_pBufferOut	= reinterpret_cast< __m128i * >( p_pDstBuffer );

				while ( l_pBufferIn != l_pBufferInEnd )
				{
					// On récupère les composantes YUV
					yuv			= _mm_load_si128( l_pLineIn++ );
					// On récupère dans Y les Y uniquement
					y			= _mm_and_si128( yuv, ymask );
					y			= _mm_sub_epi16( y, ysub );
					// dans U les U uniquement
					u			= _mm_and_si128( yuv, umask );
					// que l'on duplique pour en avoir autant que des Y
					u			= _mm_or_si128( _mm_srli_epi32( u, 8 ), _mm_slli_epi32( u, 8 ) );
					u			= _mm_sub_epi16( u, uvsub );
					// dans V les V uniquement
					v			= _mm_and_si128( yuv, vmask );
					// que l'on duplique pour en avoir autant que des Y
					v			= _mm_or_si128( _mm_srli_epi32( v, 8 ), _mm_srli_epi32( v, 24 ) );
					v			= _mm_sub_epi16( v, uvsub );
					// On calcule les facteurs finaux YUV
					y			= _mm_mullo_epi16( facy, y );
					rv0			= _mm_mullo_epi16( facrv, v );
					gu0			= _mm_mullo_epi16( facgu, u );
					gv0			= _mm_mullo_epi16( facgv, v );
					bu0			= _mm_mullo_epi16( facbu, u );
					// Maintenant on calcule les facteurs RGB
					r0			= _mm_srai_epi16( _mm_add_epi16( y, rv0 ), 6 );
					g0			= _mm_srai_epi16( _mm_sub_epi16( y, _mm_add_epi16( gu0, gv0 ) ), 6 );
					b0			= _mm_srai_epi16( _mm_add_epi16( y, bu0 ), 6 );
					// Qu'on sature (clamp entre 0 et 255)
					r0			= _mm_packus_epi16( r0, zero );		// 00000000rrrrrrrr saturated
					g0			= _mm_packus_epi16( g0, zero );		// 00000000gggggggg saturated
					b0			= _mm_packus_epi16( b0, zero );		// 00000000bbbbbbbb saturated
					// Et que l'on ordonne
					r1			= _mm_unpacklo_epi8( zero, b0 );	// b0b0b0b0b0b0b0b0
					gb			= _mm_unpacklo_epi8( g0, r0 );		// rgrgrgrgrgrgrgrg
					argb0123	= _mm_unpacklo_epi16( r1, gb );		// rgb0rgb0rgb0rgb0
					argb4567	= _mm_unpackhi_epi16( r1, gb );		// rgb0rgb0rgb0rgb0
					argb0123	= _mm_or_si128( amask, argb0123 );	// rgbFrgbFrgbFrgbF
					argb4567	= _mm_or_si128( amask, argb4567 );	// rgbFrgbFrgbFrgbF
					// Enfin on fout tout ce bordel dans le buffer
					std::memcpy( l_pBufferOut, &argb0123, sizeof( __m128i ) );
					l_pBufferOut++;
					std::memcpy( l_pBufferOut, &argb4567, sizeof( __m128i ) );
					l_pBufferOut++;
				}
			}
		};
		/*!
		\author 	Sylvain DOREMUS
		\date 		08/09/2013
		\~english
		\brief		Specialisation for YUY2 to R8G8B8 conversion
		\~french
		\brief		Spécialisation pour la conversion YUY2 vers R8G8B8
		*/
		template<>
		struct buffer_converter< PixelFormat::eYUY2, PixelFormat::eR8G8B8 >
		{
			static const __m128i umask;
			static const __m128i vmask;
			static const __m128i ymask;
			static const __m128i uvsub;
			static const __m128i ysub;
			static const __m128i facy;
			static const __m128i facrv;
			static const __m128i facgu;
			static const __m128i facgv;
			static const __m128i facbu;
			static const __m128i zero;

			inline void operator()( uint8_t const * p_pSrcBuffer, uint32_t p_uiSrcSize, uint8_t * p_pDstBuffer, uint32_t p_uiDstSize )
			{
				__m128i yuv;
				__m128i y;
				__m128i	u;
				__m128i	v;
				__m128i rv0;
				__m128i gu0;
				__m128i gv0;
				__m128i bu0;
				__m128i r0;
				__m128i g0;
				__m128i b0;
				__m128i r1;
				__m128i gb;
				__m128i argb0123;
				__m128i argb4567;
				__m128i const *	l_pBufferIn		= l_pBufferIn = reinterpret_cast< __m128i const * >( p_pSrcBuffer );
				__m128i const *	l_pBufferInEnd	= reinterpret_cast< __m128i const * >( p_pSrcBuffer + p_uiSrcSize );
				__m128i const *	l_pLineIn		= l_pBufferIn;
				uint8_t 	*	l_pBufferOut	= p_pDstBuffer;

				while ( l_pBufferIn != l_pBufferInEnd )
				{
					// On récupère les composantes YUV
					yuv			= _mm_load_si128( l_pLineIn++ );
					// On récupère dans Y les Y uniquement
					y			= _mm_and_si128( yuv, ymask );
					y			= _mm_sub_epi16( y, ysub );
					// dans U les U uniquement
					u			= _mm_and_si128( yuv, umask );
					// que l'on duplique pour en avoir autant que des Y
					u			= _mm_or_si128( _mm_srli_epi32( u, 8 ), _mm_slli_epi32( u, 8 ) );
					u			= _mm_sub_epi16( u, uvsub );
					// dans V les V uniquement
					v			= _mm_and_si128( yuv, vmask );
					// que l'on duplique pour en avoir autant que des Y
					v			= _mm_or_si128( _mm_srli_epi32( v, 8 ), _mm_srli_epi32( v, 24 ) );
					v			= _mm_sub_epi16( v, uvsub );
					// On calcule les facteurs finaux YUV
					y			= _mm_mullo_epi16( facy, y );
					rv0			= _mm_mullo_epi16( facrv, v );
					gu0			= _mm_mullo_epi16( facgu, u );
					gv0			= _mm_mullo_epi16( facgv, v );
					bu0			= _mm_mullo_epi16( facbu, u );
					// Maintenant on calcule les facteurs RGB
					r0			= _mm_srai_epi16( _mm_add_epi16( y, rv0 ), 6 );
					g0			= _mm_srai_epi16( _mm_sub_epi16( y, _mm_add_epi16( gu0, gv0 ) ), 6 );
					b0			= _mm_srai_epi16( _mm_add_epi16( y, bu0 ), 6 );
					// Qu'on sature (clamp entre 0 et 255)
					r0			= _mm_packus_epi16( r0, zero );		// 00000000rrrrrrrr saturated
					g0			= _mm_packus_epi16( g0, zero );		// 00000000gggggggg saturated
					b0			= _mm_packus_epi16( b0, zero );		// 00000000bbbbbbbb saturated
					// Et que l'on ordonne
					r1			= _mm_unpacklo_epi8( zero, b0 );	// b0b0b0b0b0b0b0b0
					gb			= _mm_unpacklo_epi8( g0, r0 );		// rgrgrgrgrgrgrgrg
					argb0123	= _mm_unpacklo_epi16( r1, gb );		// rgb0rgb0rgb0rgb0
					argb4567	= _mm_unpackhi_epi16( r1, gb );		// rgb0rgb0rgb0rgb0
					// Enfin on fout tout ce bordel dans le buffer
					argb0123 = _mm_srli_si128( argb0123, 1 );
					std::memcpy( l_pBufferOut, &argb0123, 3 );
					l_pBufferOut += 3;
					argb0123 = _mm_srli_si128( argb0123, 4 );
					std::memcpy( l_pBufferOut, &argb0123, 3 );
					l_pBufferOut += 3;
					argb0123 = _mm_srli_si128( argb0123, 4 );
					std::memcpy( l_pBufferOut, &argb0123, 3 );
					l_pBufferOut += 3;
					argb0123 = _mm_srli_si128( argb0123, 4 );
					std::memcpy( l_pBufferOut, &argb0123, 3 );
					l_pBufferOut += 3;
					argb4567 = _mm_srli_si128( argb4567, 1 );
					std::memcpy( l_pBufferOut, &argb4567, 3 );
					l_pBufferOut += 3;
					argb4567 = _mm_srli_si128( argb4567, 4 );
					std::memcpy( l_pBufferOut, &argb4567, 3 );
					l_pBufferOut += 3;
					argb4567 = _mm_srli_si128( argb4567, 4 );
					std::memcpy( l_pBufferOut, &argb4567, 3 );
					l_pBufferOut += 3;
					argb4567 = _mm_srli_si128( argb4567, 4 );
					std::memcpy( l_pBufferOut, &argb4567, 3 );
					l_pBufferOut += 3;
				}
			}
		};

#endif

		/**
		 *\~english
		 *\brief			Conversion from static source format to dynamic destination format
		 *\remarks			Only for colour pixel formats, offsets source and destination by respective pixel format sizes
		 *\param[in,out]	p_pSrc			The source pixel
		 *\param[in,out]	p_pDst			The destination pixel
		 *\param[in]		p_ePixelFmtDst	The destination pixel format
		 *\~french
		 *\brief			Conversion d'une source statique vers une destination dynamique
		 *\remarks			Uniquement pour les formats de pixel couleurs, décale la source et la destination par la taille respective de leur format de pixel
		 *\param[in,out]	p_pSrc			Le pixel source
		 *\param[in,out]	p_pDst			Le pixel destination
		 *\param[in]		p_ePixelFmtDst	Le format du pixel destination
		 */
		template< PixelFormat PF >
		void DynamicColourConversion( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			switch ( p_ePixelFmtDst )
			{
			case PixelFormat::eL8:
				pixel_converter< PF, PixelFormat::eL8 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eL16F32F:
				pixel_converter< PF, PixelFormat::eL16F32F >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eL32F:
				pixel_converter< PF, PixelFormat::eL32F >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eA8L8:
				pixel_converter< PF, PixelFormat::eA8L8 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eAL16F32F:
				pixel_converter< PF, PixelFormat::eAL16F32F >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eAL32F:
				pixel_converter< PF, PixelFormat::eAL32F >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eA1R5G5B5:
				pixel_converter< PF, PixelFormat::eA1R5G5B5 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eA4R4G4B4:
				pixel_converter< PF, PixelFormat::eA4R4G4B4 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eR5G6B5:
				pixel_converter< PF, PixelFormat::eR5G6B5 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eR8G8B8:
				pixel_converter< PF, PixelFormat::eR8G8B8 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eB8G8R8:
				pixel_converter< PF, PixelFormat::eB8G8R8 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eA8R8G8B8:
				pixel_converter< PF, PixelFormat::eA8R8G8B8 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eA8B8G8R8:
				pixel_converter< PF, PixelFormat::eA8B8G8R8 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eRGB16F:
				pixel_converter< PF, PixelFormat::eRGB16F >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eRGBA16F:
				pixel_converter< PF, PixelFormat::eRGBA16F >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eRGB16F32F:
				pixel_converter< PF, PixelFormat::eRGB16F32F >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eRGBA16F32F:
				pixel_converter< PF, PixelFormat::eRGBA16F32F >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eRGB32F:
				pixel_converter< PF, PixelFormat::eRGB32F >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eRGBA32F:
				pixel_converter< PF, PixelFormat::eRGBA32F >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eYUY2:
				pixel_converter< PF, PixelFormat::eYUY2 >()( p_pSrc, p_pDst );
				break;

			default:
				UNSUPPORTED_ERROR( "No conversion defined" );
				break;
			}
		}
		/**
		 *\~english
		 *\brief			Conversion from static source format to dynamic destination format
		 *\remarks			Only for depth pixel formats, offsets source and destination by respective pixel format sizes
		 *\param[in,out]	p_pSrc			The source pixel
		 *\param[in,out]	p_pDst			The destination pixel
		 *\param[in]		p_ePixelFmtDst	The destination pixel format
		 *\~french
		 *\brief			Conversion d'une source statique vers une destination dynamique
		 *\remarks			Uniquement pour les formats de pixel profondeurs, décale la source et la destination par la taille respective de leur format de pixel
		 *\param[in,out]	p_pSrc			Le pixel source
		 *\param[in,out]	p_pDst			Le pixel destination
		 *\param[in]		p_ePixelFmtDst	Le format du pixel destination
		 */
		template< PixelFormat PF >
		void DynamicDepthConversion( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			switch ( p_ePixelFmtDst )
			{
			case PixelFormat::eD16:
				pixel_converter< PF, PixelFormat::eD16 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eD24:
				pixel_converter< PF, PixelFormat::eD24 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eD24S8:
				pixel_converter< PF, PixelFormat::eD24S8 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eD32:
				pixel_converter< PF, PixelFormat::eD32 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eD32F:
				pixel_converter< PF, PixelFormat::eD32F >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eD32FS8:
				pixel_converter< PF, PixelFormat::eD32FS8 >()( p_pSrc, p_pDst );
				break;

			default:
				UNSUPPORTED_ERROR( "No conversion defined" );
				break;
			}
		}
		/**
		 *\~english
		 *\brief			Conversion from static source format to dynamic destination format
		 *\remarks			Only for stencil pixel formats, offsets source and destination by respective pixel format sizes
		 *\param[in,out]	p_pSrc			The source pixel
		 *\param[in,out]	p_pDst			The destination pixel
		 *\param[in]		p_ePixelFmtDst	The destination pixel format
		 *\~french
		 *\brief			Conversion d'une source statique vers une destination dynamique
		 *\remarks			Uniquement pour les formats de pixel stencil, décale la source et la destination par la taille respective de leur format de pixel
		 *\param[in,out]	p_pSrc			Le pixel source
		 *\param[in,out]	p_pDst			Le pixel destination
		 *\param[in]		p_ePixelFmtDst	Le format du pixel destination
		 */
		template< PixelFormat PF >
		void DynamicStencilConversion( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			switch ( p_ePixelFmtDst )
			{
			case PixelFormat::eD24S8:
				pixel_converter< PF, PixelFormat::eD24S8 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eS1:
				pixel_converter< PF, PixelFormat::eS1 >()( p_pSrc, p_pDst );
				break;

			case PixelFormat::eS8:
				pixel_converter< PF, PixelFormat::eS8 >()( p_pSrc, p_pDst );
				break;

			default:
				UNSUPPORTED_ERROR( "No conversion defined" );
				break;
			}
		}
		/**
		 *\~english
		 *\brief		Buffer conversion
		 *\param[in]	p_pSrcBuffer	The source buffer
		 *\param[in]	p_uiSrcSize		The source buffer size
		 *\param[in]	p_eDstFormat	The destination pixel format
		 *\param[in]	p_pDstBuffer	The destination buffer
		 *\param[in]	p_uiDstSize		The destination buffer size
		 *\~french
		 *\brief		Conversion de buffer
		 *\param[in]	p_pSrcBuffer	Le buffer source
		 *\param[in]	p_uiSrcSize		La taille du buffer source
		 *\param[in]	p_eDstFormat	Le format du buffer destination
		 *\param[in]	p_pDstBuffer	Le buffer destination
		 *\param[in]	p_uiDstSize		La taille du buffer destination
		 */
		template< PixelFormat PF >
		void DynamicColourBufferConversion( uint8_t const * p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t * p_pDstBuffer, uint32_t p_uiDstSize )
		{
			switch ( p_eDstFormat )
			{
			case PixelFormat::eL8:
				buffer_converter< PF, PixelFormat::eL8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eL16F32F:
				buffer_converter< PF, PixelFormat::eL16F32F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eL32F:
				buffer_converter< PF, PixelFormat::eL32F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eA8L8:
				buffer_converter< PF, PixelFormat::eA8L8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eAL16F32F:
				buffer_converter< PF, PixelFormat::eAL16F32F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eAL32F:
				buffer_converter< PF, PixelFormat::eAL32F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eA1R5G5B5:
				buffer_converter< PF, PixelFormat::eA1R5G5B5 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eA4R4G4B4:
				buffer_converter< PF, PixelFormat::eA4R4G4B4 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eR5G6B5:
				buffer_converter< PF, PixelFormat::eR5G6B5 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eR8G8B8:
				buffer_converter< PF, PixelFormat::eR8G8B8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eB8G8R8:
				buffer_converter< PF, PixelFormat::eB8G8R8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eA8R8G8B8:
				buffer_converter< PF, PixelFormat::eA8R8G8B8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eA8B8G8R8:
				buffer_converter< PF, PixelFormat::eA8B8G8R8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eRGB16F:
				buffer_converter< PF, PixelFormat::eRGB16F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eRGBA16F:
				buffer_converter< PF, PixelFormat::eRGBA16F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eRGB16F32F:
				buffer_converter< PF, PixelFormat::eRGB16F32F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eRGBA16F32F:
				buffer_converter< PF, PixelFormat::eRGBA16F32F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eRGB32F:
				buffer_converter< PF, PixelFormat::eRGB32F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eRGBA32F:
				buffer_converter< PF, PixelFormat::eRGBA32F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eYUY2:
				buffer_converter< PF, PixelFormat::eYUY2 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			default:
				UNSUPPORTED_ERROR( "No conversion defined" );
				break;
			}
		}
		/**
		 *\~english
		 *\brief		Buffer conversion
		 *\param[in]	p_pSrcBuffer	The source buffer
		 *\param[in]	p_uiSrcSize		The source buffer size
		 *\param[in]	p_eDstFormat	The destination pixel format
		 *\param[in]	p_pDstBuffer	The destination buffer
		 *\param[in]	p_uiDstSize		The destination buffer size
		 *\~french
		 *\brief		Conversion de buffer
		 *\param[in]	p_pSrcBuffer	Le buffer source
		 *\param[in]	p_uiSrcSize		La taille du buffer source
		 *\param[in]	p_eDstFormat	Le format du buffer destination
		 *\param[in]	p_pDstBuffer	Le buffer destination
		 *\param[in]	p_uiDstSize		La taille du buffer destination
		 */
		template< PixelFormat PF >
		void DynamicDepthBufferConversion( uint8_t const * p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t * p_pDstBuffer, uint32_t p_uiDstSize )
		{
			switch ( p_eDstFormat )
			{
			case PixelFormat::eD16:
				buffer_converter< PF, PixelFormat::eD16 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eD24:
				buffer_converter< PF, PixelFormat::eD24 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eD24S8:
				buffer_converter< PF, PixelFormat::eD24S8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eD32:
				buffer_converter< PF, PixelFormat::eD32 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eD32F:
				buffer_converter< PF, PixelFormat::eD32F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eD32FS8:
				buffer_converter< PF, PixelFormat::eD32FS8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eA8R8G8B8:
				buffer_converter< PF, PixelFormat::eA8R8G8B8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eRGB32F:
				buffer_converter< PF, PixelFormat::eRGB32F >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eL8:
				buffer_converter< PF, PixelFormat::eL8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			default:
				UNSUPPORTED_ERROR( "No conversion defined" );
				break;
			}
		}
		/**
		 *\~english
		 *\brief		Buffer conversion
		 *\param[in]	p_pSrcBuffer	The source buffer
		 *\param[in]	p_uiSrcSize		The source buffer size
		 *\param[in]	p_eDstFormat	The destination pixel format
		 *\param[in]	p_pDstBuffer	The destination buffer
		 *\param[in]	p_uiDstSize		The destination buffer size
		 *\~french
		 *\brief		Conversion de buffer
		 *\param[in]	p_pSrcBuffer	Le buffer source
		 *\param[in]	p_uiSrcSize		La taille du buffer source
		 *\param[in]	p_eDstFormat	Le format du buffer destination
		 *\param[in]	p_pDstBuffer	Le buffer destination
		 *\param[in]	p_uiDstSize		La taille du buffer destination
		 */
		template< PixelFormat PF >
		void DynamicStencilBufferConversion( uint8_t const * p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t * p_pDstBuffer, uint32_t p_uiDstSize )
		{
			switch ( p_eDstFormat )
			{
			case PixelFormat::eD24S8:
				buffer_converter< PF, PixelFormat::eD24S8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eS1:
				buffer_converter< PF, PixelFormat::eS1 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			case PixelFormat::eS8:
				buffer_converter< PF, PixelFormat::eS8 >()( p_pSrcBuffer, p_uiSrcSize, p_pDstBuffer, p_uiDstSize );
				break;

			default:
				UNSUPPORTED_ERROR( "No conversion defined" );
				break;
			}
		}
	}

	//*************************************************************************************************

	namespace PF
	{
		namespace
		{
			template< PixelFormat PF, typename Enable = void > struct PixelComponents;

			template< PixelFormat PF >
			struct PixelComponents< PF, typename std::enable_if< is_colour_format< PF >::value >::type >
			{
				static float GetFloat( Pixel< PF > const & p_pixel, PixelComponent p_component )
				{
					float l_result = 0.0f;

					switch ( p_component )
					{
					case PixelComponent::eRed:
						l_result = component< PF >::R32F( p_pixel.const_ptr() );
						break;

					case PixelComponent::eGreen:
						l_result = component< PF >::G32F( p_pixel.const_ptr() );
						break;

					case PixelComponent::eBlue:
						l_result = component< PF >::B32F( p_pixel.const_ptr() );
						break;

					case PixelComponent::eAlpha:
						l_result = component< PF >::A32F( p_pixel.const_ptr() );
						break;

					case PixelComponent::eLuminance:
						l_result = component< PF >::L32F( p_pixel.const_ptr() );
						break;

					default:
						l_result = 0;
						break;
					}

					return l_result;
				}

				static void SetFloat( Pixel< PF > & p_pixel, PixelComponent p_component, float p_value )
				{
					switch ( p_component )
					{
					case PixelComponent::eRed:
						component< PF >::R8( p_pixel.ptr(), p_value );
						break;

					case PixelComponent::eGreen:
						component< PF >::G8( p_pixel.ptr(), p_value );
						break;

					case PixelComponent::eBlue:
						component< PF >::B8( p_pixel.ptr(), p_value );
						break;

					case PixelComponent::eAlpha:
						component< PF >::A8( p_pixel.ptr(), p_value );
						break;

					case PixelComponent::eLuminance:
						component< PF >::L8( p_pixel.ptr(), p_value );
						break;

					default:
						break;
					}
				}

				static uint8_t GetByte( Pixel< PF > const & p_pixel, PixelComponent p_component )
				{
					uint8_t l_result = 0;

					switch ( p_component )
					{
					case PixelComponent::eRed:
						l_result = component< PF >::R8( p_pixel.const_ptr() );
						break;

					case PixelComponent::eGreen:
						l_result = component< PF >::G8( p_pixel.const_ptr() );
						break;

					case PixelComponent::eBlue:
						l_result = component< PF >::B8( p_pixel.const_ptr() );
						break;

					case PixelComponent::eAlpha:
						l_result = component< PF >::A8( p_pixel.const_ptr() );
						break;

					case PixelComponent::eLuminance:
						l_result = component< PF >::L8( p_pixel.const_ptr() );
						break;

					default:
						l_result = 0;
						break;
					}

					return l_result;
				}

				static void SetByte( Pixel< PF > & p_pixel, PixelComponent p_component, uint8_t p_value )
				{
					switch ( p_component )
					{
					case PixelComponent::eRed:
						component< PF >::R8( p_pixel.ptr(), p_value );
						break;

					case PixelComponent::eGreen:
						component< PF >::G8( p_pixel.ptr(), p_value );
						break;

					case PixelComponent::eBlue:
						component< PF >::B8( p_pixel.ptr(), p_value );
						break;

					case PixelComponent::eAlpha:
						component< PF >::A8( p_pixel.ptr(), p_value );
						break;

					case PixelComponent::eLuminance:
						component< PF >::L8( p_pixel.ptr(), p_value );
						break;

					default:
						break;
					}
				}
			};

			template< PixelFormat PF >
			struct PixelComponents< PF, typename std::enable_if< is_depth_stencil_format< PF >::value >::type >
			{
				static float GetFloat( Pixel< PF > const & p_pixel, PixelComponent p_component )
				{
					float l_result = 0.0f;

					switch ( p_component )
					{
					case PixelComponent::eDepth:
						l_result = float( component< PF >::D32F( p_pixel.const_ptr() ) );
						break;

					case PixelComponent::eStencil:
						l_result = float( component< PF >::S32F( p_pixel.const_ptr() ) );
						break;

					default:
						l_result = 0;
						break;
					}

					return l_result;
				}

				static void SetFloat( Pixel< PF > & p_pixel, PixelComponent p_component, float p_value )
				{
					switch ( p_component )
					{
					case PixelComponent::eDepth:
						component< PF >::D32F( p_pixel.ptr(), p_value );
						break;

					case PixelComponent::eStencil:
						component< PF >::S32F( p_pixel.ptr(), p_value );
						break;

					default:
						break;
					}
				}

				static uint8_t GetByte( Pixel< PF > const & p_pixel, PixelComponent p_component )
				{
					uint8_t l_result = 0;

					switch ( p_component )
					{
					case PixelComponent::eDepth:
						l_result = uint8_t( component< PF >::D32( p_pixel.const_ptr() ) >> 24 );
						break;

					case PixelComponent::eStencil:
						l_result = uint8_t( component< PF >::S8( p_pixel.const_ptr() ) );
						break;

					default:
						l_result = 0;
						break;
					}

					return l_result;
				}

				static void SetByte( Pixel< PF > & p_pixel, PixelComponent p_component, uint8_t p_value )
				{
					switch ( p_component )
					{
					case PixelComponent::eDepth:
						component< PF >::D32( p_pixel.ptr(), uint32_t( p_value ) << 24 );
						break;

					case PixelComponent::eStencil:
						component< PF >::S8( p_pixel.ptr(), p_value );
						break;

					default:
						break;
					}
				}

				static uint16_t GetUInt16( Pixel< PF > const & p_pixel, PixelComponent p_component )
				{
					uint32_t l_result = 0;

					switch ( p_component )
					{
					case PixelComponent::eDepth:
						l_result = component< PF >::D16( p_pixel.const_ptr() );
						break;

					default:
						l_result = 0;
						break;
					}

					return l_result;
				}

				static void SetUInt16( Pixel< PF > & p_pixel, PixelComponent p_component, uint16_t p_value )
				{
					switch ( p_component )
					{
					case PixelComponent::eDepth:
						component< PF >::D16( p_pixel.ptr(), p_value );
						break;

					default:
						break;
					}
				}

				static uint32_t GetUInt24( Pixel< PF > const & p_pixel, PixelComponent p_component )
				{
					uint32_t l_result = 0;

					switch ( p_component )
					{
					case PixelComponent::eDepth:
						l_result = component< PF >::D24( p_pixel.const_ptr() );
						break;

					default:
						l_result = 0;
						break;
					}

					return l_result;
				}

				static void SetUInt24( Pixel< PF > & p_pixel, PixelComponent p_component, uint32_t p_value )
				{
					switch ( p_component )
					{
					case PixelComponent::eDepth:
						component< PF >::D24( p_pixel.ptr(), p_value );
						break;

					default:
						break;
					}
				}

				static uint32_t GetUInt32( Pixel< PF > const & p_pixel, PixelComponent p_component )
				{
					uint32_t l_result = 0;

					switch ( p_component )
					{
					case PixelComponent::eDepth:
						l_result = component< PF >::D32( p_pixel.const_ptr() );
						break;

					default:
						l_result = 0;
						break;
					}

					return l_result;
				}

				static void SetUInt32( Pixel< PF > & p_pixel, PixelComponent p_component, uint32_t p_value )
				{
					switch ( p_component )
					{
					case PixelComponent::eDepth:
						component< PF >::D32( p_pixel.ptr(), p_value );
						break;

					default:
						break;
					}
				}
			};
		}

		template< PixelFormat PF >
		float GetFloatComponent( Pixel< PF > const & p_pixel, PixelComponent p_component )
		{
			return PixelComponents< PF >::GetFloat( p_pixel, p_component );
		}

		template< PixelFormat PF >
		void SetFloatComponent( Pixel< PF > & p_pixel, PixelComponent p_component, float p_value )
		{
			return PixelComponents< PF >::SetFloat( p_pixel, p_component, p_value );
		}

		template< PixelFormat PF >
		uint8_t GetByteComponent( Pixel< PF > const & p_pixel, PixelComponent p_component )
		{
			return PixelComponents< PF >::GetByte( p_pixel, p_component );
		}

		template< PixelFormat PF >
		void SetByteComponent( Pixel< PF > & p_pixel, PixelComponent p_component, uint8_t p_value )
		{
			return PixelComponents< PF >::SetByte( p_pixel, p_component, p_value );
		}

		template< PixelFormat PF >
		uint16_t GetUInt16Component( Pixel< PF > const & p_pixel, PixelComponent p_component )
		{
			return PixelComponents< PF >::GetUInt16( p_pixel, p_component );
		}

		template< PixelFormat PF >
		void SetUInt16Component( Pixel< PF > & p_pixel, PixelComponent p_component, uint16_t p_value )
		{
			return PixelComponents< PF >::SetUInt16( p_pixel, p_component, p_value );
		}

		template< PixelFormat PF >
		uint32_t GetUInt24Component( Pixel< PF > const & p_pixel, PixelComponent p_component )
		{
			return PixelComponents< PF >::GetUInt24( p_pixel, p_component );
		}

		template< PixelFormat PF >
		void SetUInt24Component( Pixel< PF > & p_pixel, PixelComponent p_component, uint32_t p_value )
		{
			return PixelComponents< PF >::SetUInt24( p_pixel, p_component, p_value );
		}

		template< PixelFormat PF >
		uint32_t GetUInt32Component( Pixel< PF > const & p_pixel, PixelComponent p_component )
		{
			return PixelComponents< PF >::GetUInt32( p_pixel, p_component );
		}

		template< PixelFormat PF >
		void SetUInt32Component( Pixel< PF > & p_pixel, PixelComponent p_component, uint32_t p_value )
		{
			return PixelComponents< PF >::SetUInt32( p_pixel, p_component, p_value );
		}
	}

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eL8
	//!\~french		Spécialisation pour PixelFormat::eL8
	template<>
	struct pixel_definitions< PixelFormat::eL8 >
	{
		static const uint8_t Size = 1;
		static const uint8_t Count = 1;
		static const bool Alpha = false;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "8 bits luminosity" );
		}
		static inline String to_str()
		{
			return cuT( "l8" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eL8 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eL8 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eL8, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eL16F32F
	//!\~french		Spécialisation pour PixelFormat::eL16F32F
	template<>
	struct pixel_definitions< PixelFormat::eL16F32F >
	{
		static const uint8_t Size = 4;
		static const uint8_t Count = 2;
		static const bool Alpha = false;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "Half floats luminosity" );
		}
		static inline String to_str()
		{
			return cuT( "l16f" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eL16F32F >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eL16F32F >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eL16F32F, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eL32F
	//!\~french		Spécialisation pour PixelFormat::eL32F
	template<>
	struct pixel_definitions< PixelFormat::eL32F >
	{
		static const uint8_t Size = 4;
		static const uint8_t Count = 1;
		static const bool Alpha = false;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "Floats luminosity" );
		}
		static inline String to_str()
		{
			return cuT( "l32f" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eL32F >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eL32F >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eL32F, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eA8L8
	//!\~french		Spécialisation pour PixelFormat::eA8L8
	template<>
	struct pixel_definitions< PixelFormat::eA8L8 >
	{
		static const uint8_t Size = 2;
		static const uint8_t Count = 2;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static const PixelFormat NoAlphaPF = PixelFormat::eL8;
		static inline String to_string()
		{
			return cuT( "16 bits luminosity and alpha" );
		}
		static inline String to_str()
		{
			return cuT( "al16" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eA8L8 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eA8L8 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eA8L8, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eAL32F
	//!\~french		Spécialisation pour PixelFormat::eAL32F
	template<>
	struct pixel_definitions< PixelFormat::eAL32F >
	{
		static const uint8_t Size = 8;
		static const uint8_t Count = 2;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static const PixelFormat NoAlphaPF = PixelFormat::eL32F;
		static inline String to_string()
		{
			return cuT( "Floats luminosity and alpha" );
		}
		static inline String to_str()
		{
			return cuT( "al32f" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eAL32F >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eAL32F >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eAL32F, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eAL16F32F
	//!\~french		Spécialisation pour PixelFormat::eAL16F32F
	template<>
	struct pixel_definitions< PixelFormat::eAL16F32F >
	{
		static const uint8_t Size = 8;
		static const uint8_t Count = 2;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static const PixelFormat NoAlphaPF = PixelFormat::eL16F32F;
		static inline String to_string()
		{
			return cuT( "Half floats luminosity and alpha" );
		}
		static inline String to_str()
		{
			return cuT( "al16f" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eAL16F32F >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eAL16F32F >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eAL16F32F, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eA1R5G5B5
	//!\~french		Spécialisation pour PixelFormat::eA1R5G5B5
	template<>
	struct pixel_definitions< PixelFormat::eA1R5G5B5 >
	{
		static const uint8_t Size = 2;
		static const uint8_t Count = 4;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static const PixelFormat NoAlphaPF = PixelFormat::eR5G6B5;
		static inline String to_string()
		{
			return cuT( "16 bits 5551 ARGB" );
		}
		static inline String to_str()
		{
			return cuT( "argb1555" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eA1R5G5B5 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eA1R5G5B5 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eA1R5G5B5, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eR5G6B5
	//!\~french		Spécialisation pour PixelFormat::eR5G6B5
	template<>
	struct pixel_definitions< PixelFormat::eR5G6B5 >
	{
		static const uint8_t Size = 2;
		static const uint8_t Count = 3;
		static const bool Alpha = false;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "16 bits 565 RGB" );
		}
		static inline String to_str()
		{
			return cuT( "rgb565" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eR5G6B5 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eR5G6B5 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eR5G6B5, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eA4R4G4B4
	//!\~french		Spécialisation pour PixelFormat::eA4R4G4B4
	template<>
	struct pixel_definitions< PixelFormat::eA4R4G4B4 >
	{
		static const uint8_t Size = 2;
		static const uint8_t Count = 4;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static const PixelFormat NoAlphaPF = PixelFormat::eR5G6B5;
		static inline String to_string()
		{
			return cuT( "16 bits 4444 ARGB" );
		}
		static inline String to_str()
		{
			return cuT( "argb16" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eA4R4G4B4 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eA4R4G4B4 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eA4R4G4B4, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eR8G8B8
	//!\~french		Spécialisation pour PixelFormat::eR8G8B8
	template<>
	struct pixel_definitions< PixelFormat::eR8G8B8 >
	{
		static const uint8_t Size = 3;
		static const uint8_t Count = 3;
		static const bool Alpha = false;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "24 bits 888 RGB" );
		}
		static inline String to_str()
		{
			return cuT( "rgb24" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eR8G8B8 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eR8G8B8 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eR8G8B8, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eB8G8R8
	//!\~french		Spécialisation pour PixelFormat::eB8G8R8
	template<>
	struct pixel_definitions< PixelFormat::eB8G8R8 >
	{
		static const uint8_t Size = 3;
		static const uint8_t Count = 3;
		static const bool Alpha = false;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "24 bits 888 BGR" );
		}
		static inline String to_str()
		{
			return cuT( "bgr24" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eB8G8R8 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eB8G8R8 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eB8G8R8, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eA8R8G8B8
	//!\~french		Spécialisation pour PixelFormat::eA8R8G8B8
	template<>
	struct pixel_definitions< PixelFormat::eA8R8G8B8 >
	{
		static const uint8_t Size = 4;
		static const uint8_t Count = 4;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static const PixelFormat NoAlphaPF = PixelFormat::eR8G8B8;
		static inline String to_string()
		{
			return cuT( "32 bits 8888 ARGB" );
		}
		static inline String to_str()
		{
			return cuT( "argb32" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eA8R8G8B8 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eA8R8G8B8 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eA8R8G8B8, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eA8B8G8R8
	//!\~french		Spécialisation pour PixelFormat::eA8B8G8R8
	template<>
	struct pixel_definitions< PixelFormat::eA8B8G8R8 >
	{
		static const uint8_t Size = 4;
		static const uint8_t Count = 4;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static const PixelFormat NoAlphaPF = PixelFormat::eB8G8R8;
		static inline String to_string()
		{
			return cuT( "32 bits 8888 ABGR" );
		}
		static inline String to_str()
		{
			return cuT( "abgr32" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eA8B8G8R8 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eA8B8G8R8 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eA8B8G8R8, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eRGB16F
	//!\~french		Spécialisation pour PixelFormat::eRGB16F
	template<>
	struct pixel_definitions< PixelFormat::eRGB16F >
	{
		static const uint8_t Size = 6;
		static const uint8_t Count = 3;
		static const bool Alpha = false;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "Half floating point RGB" );
		}
		static inline String to_str()
		{
			return cuT( "rgb16f" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eRGB16F >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eRGB16F >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eRGB16F, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eRGBA16F
	//!\~french		Spécialisation pour PixelFormat::eRGBA16F
	template<>
	struct pixel_definitions< PixelFormat::eRGBA16F >
	{
		static const uint8_t Size = 8;
		static const uint8_t Count = 4;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static const PixelFormat NoAlphaPF = PixelFormat::eRGB16F;
		static inline String to_string()
		{
			return cuT( "Half floating point ARGB" );
		}
		static inline String to_str()
		{
			return cuT( "argb16f" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eRGBA16F >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eRGBA16F >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eRGBA16F, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eRGB16F32F
	//!\~french		Spécialisation pour PixelFormat::eRGB16F32F
	template<>
	struct pixel_definitions< PixelFormat::eRGB16F32F >
	{
		static const uint8_t Size = 12;
		static const uint8_t Count = 3;
		static const bool Alpha = false;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "GPU half floating point RGB, CPU floating point RGB" );
		}
		static inline String to_str()
		{
			return cuT( "rgb16f32f" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eRGB32F >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eRGB32F >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eRGB32F, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eRGBA16F32F
	//!\~french		Spécialisation pour PixelFormat::eRGBA16F32F
	template<>
	struct pixel_definitions< PixelFormat::eRGBA16F32F >
	{
		static const uint8_t Size = 16;
		static const uint8_t Count = 4;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static const PixelFormat NoAlphaPF = PixelFormat::eRGB16F32F;
		static inline String to_string()
		{
			return cuT( "GPU half floating point ARGB, CPU floating point ARGB" );
		}
		static inline String to_str()
		{
			return cuT( "argb16f32f" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eRGBA32F >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eRGBA32F >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eRGBA32F, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eRGB32F
	//!\~french		Spécialisation pour PixelFormat::eRGB32F
	template<>
	struct pixel_definitions< PixelFormat::eRGB32F >
	{
		static const uint8_t Size = 12;
		static const uint8_t Count = 3;
		static const bool Alpha = false;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "Floating point RGB" );
		}
		static inline String to_str()
		{
			return cuT( "rgb32f" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eRGB32F >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eRGB32F >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eRGB32F, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eRGBA32F
	//!\~french		Spécialisation pour PixelFormat::eRGBA32F
	template<>
	struct pixel_definitions< PixelFormat::eRGBA32F >
	{
		static const uint8_t Size = 16;
		static const uint8_t Count = 4;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static const PixelFormat NoAlphaPF = PixelFormat::eRGB32F;
		static inline String to_string()
		{
			return cuT( "Floating point ARGB" );
		}
		static inline String to_str()
		{
			return cuT( "argb32f" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eRGBA32F >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eRGBA32F >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eRGBA32F, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eDXTC1
	//!\~french		Spécialisation pour PixelFormat::eDXTC1
	template<>
	struct pixel_definitions< PixelFormat::eDXTC1 >
	{
		static const uint8_t Size = 1;
		static const uint8_t Count = 3;
		static const bool Alpha = false;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = true;
		static inline String to_string()
		{
			return cuT( "DXT1 8 bits compressed format" );
		}
		static inline String to_str()
		{
			return cuT( "dxtc1" );
		}
		static inline void convert( uint8_t const *, uint8_t *, PixelFormat )
		{
			UNSUPPORTED_ERROR( "No conversion from " + string::string_cast< char >( to_string() ) + " defined" );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			UNSUPPORTED_ERROR( "No conversion from " + string::string_cast< char >( to_string() ) + " defined" );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eDXTC1, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eDXTC3
	//!\~french		Spécialisation pour PixelFormat::eDXTC3
	template<>
	struct pixel_definitions< PixelFormat::eDXTC3 >
	{
		static const uint8_t Size = 2;
		static const uint8_t Count = 4;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = true;
		static const PixelFormat NoAlphaPF = PixelFormat::eDXTC3;
		static inline String to_string()
		{
			return cuT( "DXT3 16 bits compressed format" );
		}
		static inline String to_str()
		{
			return cuT( "dxtc3" );
		}
		static inline void convert( uint8_t const *, uint8_t *, PixelFormat )
		{
			UNSUPPORTED_ERROR( "No conversion from " + string::string_cast< char >( to_string() ) + " defined" );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			UNSUPPORTED_ERROR( "No conversion from " + string::string_cast< char >( to_string() ) + " defined" );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eDXTC3, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eDXTC5
	//!\~french		Spécialisation pour PixelFormat::eDXTC5
	template<>
	struct pixel_definitions< PixelFormat::eDXTC5 >
	{
		static const uint8_t Size = 2;
		static const uint8_t Count = 4;
		static const bool Alpha = true;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = true;
		static const PixelFormat NoAlphaPF = PixelFormat::eDXTC5;
		static inline String to_string()
		{
			return cuT( "DXT5 16 bits compressed format" );
		}
		static inline String to_str()
		{
			return cuT( "dxtc5" );
		}
		static inline void convert( uint8_t const *, uint8_t *, PixelFormat )
		{
			UNSUPPORTED_ERROR( "No conversion from " + string::string_cast< char >( to_string() ) + " defined" );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			UNSUPPORTED_ERROR( "No conversion from " + string::string_cast< char >( to_string() ) + " defined" );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eDXTC5, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eYUY2
	//!\~french		Spécialisation pour PixelFormat::eYUY2
	template<>
	struct pixel_definitions< PixelFormat::eYUY2 >
	{
		static const uint8_t Size = 2;
		static const uint8_t Count = 3;
		static const bool Alpha = false;
		static const bool Colour = true;
		static const bool Depth = false;
		static const bool Stencil = false;
		static const bool Compressed = true;
		static inline String to_string()
		{
			return cuT( "YUY2(4:2:2) 16 bits compressed format" );
		}
		static inline String to_str()
		{
			return cuT( "yuy2" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicColourConversion< PixelFormat::eYUY2 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicColourBufferConversion< PixelFormat::eYUY2 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eYUY2, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eD16
	//!\~french		Spécialisation pour PixelFormat::eD16
	template<>
	struct pixel_definitions< PixelFormat::eD16 >
	{
		static const uint8_t Size = 2;
		static const uint8_t Count = 1;
		static const bool Alpha = false;
		static const bool Colour = false;
		static const bool Depth = true;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "16 bits depth" );
		}
		static inline String to_str()
		{
			return cuT( "depth16" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicDepthConversion< PixelFormat::eD16 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicDepthBufferConversion< PixelFormat::eD16 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eD16, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eD24
	//!\~french		Spécialisation pour PixelFormat::eD24
	template<>
	struct pixel_definitions< PixelFormat::eD24 >
	{
		static const uint8_t Size = 3;
		static const uint8_t Count = 1;
		static const bool Alpha = false;
		static const bool Colour = false;
		static const bool Depth = true;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "24 bits depth" );
		}
		static inline String to_str()
		{
			return cuT( "depth24" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicDepthConversion< PixelFormat::eD24 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicDepthBufferConversion< PixelFormat::eD24 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eD24, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eD24S8
	//!\~french		Spécialisation pour PixelFormat::eD24S8
	template<>
	struct pixel_definitions< PixelFormat::eD24S8 >
	{
		static const uint8_t Size = 4;
		static const uint8_t Count = 2;
		static const bool Alpha = false;
		static const bool Colour = false;
		static const bool Depth = true;
		static const bool Stencil = true;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "24 bits depth, 8 bits stencil" );
		}
		static inline String to_str()
		{
			return cuT( "depth24s8" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicDepthConversion< PixelFormat::eD24S8 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			if ( p_eDstFormat == PixelFormat::eS8 || p_eDstFormat == PixelFormat::eS1 )
			{
				detail::DynamicStencilBufferConversion< PixelFormat::eD24S8 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
			}
			else
			{
				detail::DynamicDepthBufferConversion< PixelFormat::eD24S8 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
			}
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter<PixelFormat::eD24S8, PF>()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eD32
	//!\~french		Spécialisation pour PixelFormat::eD32
	template<>
	struct pixel_definitions< PixelFormat::eD32 >
	{
		static const uint8_t Size = 4;
		static const uint8_t Count = 1;
		static const bool Alpha = false;
		static const bool Colour = false;
		static const bool Depth = true;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "32 bits depth" );
		}
		static inline String to_str()
		{
			return cuT( "depth32" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicDepthConversion< PixelFormat::eD32 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicDepthBufferConversion< PixelFormat::eD32 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eD32, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eD32F
	//!\~french		Spécialisation pour PixelFormat::eD32F
	template<>
	struct pixel_definitions< PixelFormat::eD32F >
	{
		static const uint8_t Size = 4;
		static const uint8_t Count = 1;
		static const bool Alpha = false;
		static const bool Colour = false;
		static const bool Depth = true;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "32 bits floating point depth" );
		}
		static inline String to_str()
		{
			return cuT( "depth32f" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicDepthConversion< PixelFormat::eD32F >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicDepthBufferConversion< PixelFormat::eD32F >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eD32F, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eD32FS8
	//!\~french		Spécialisation pour PixelFormat::eD32FS8
	template<>
	struct pixel_definitions< PixelFormat::eD32FS8 >
	{
		static const uint8_t Size = 4;
		static const uint8_t Count = 2;
		static const bool Alpha = false;
		static const bool Colour = false;
		static const bool Depth = true;
		static const bool Stencil = false;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "32 bits floating point depth, 8 bits stencil" );
		}
		static inline String to_str()
		{
			return cuT( "depth32fs8" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicDepthConversion< PixelFormat::eD32FS8 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicDepthBufferConversion< PixelFormat::eD32FS8 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eD32FS8, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eS1
	//!\~french		Spécialisation pour PixelFormat::eS1
	template<>
	struct pixel_definitions< PixelFormat::eS1 >
	{
		static const uint8_t Size = 1;
		static const uint8_t Count = 1;
		static const bool Alpha = false;
		static const bool Colour = false;
		static const bool Depth = false;
		static const bool Stencil = true;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "1 bit stencil" );
		}
		static inline String to_str()
		{
			return cuT( "stencil1" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicStencilConversion< PixelFormat::eS1 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicStencilBufferConversion< PixelFormat::eS1 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eS1, PF >()( p_pSrc, p_pDst );
		}
	};

	//*************************************************************************************************

	//!\~english	Specialisation for PixelFormat::eS8
	//!\~french		Spécialisation pour PixelFormat::eS8
	template<>
	struct pixel_definitions< PixelFormat::eS8 >
	{
		static const uint8_t Size = 1;
		static const uint8_t Count = 1;
		static const bool Alpha = false;
		static const bool Colour = false;
		static const bool Depth = false;
		static const bool Stencil = true;
		static const bool Compressed = false;
		static inline String to_string()
		{
			return cuT( "8 bits stencil" );
		}
		static inline String to_str()
		{
			return cuT( "stencil8" );
		}
		static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst, PixelFormat p_ePixelFmtDst )
		{
			detail::DynamicStencilConversion< PixelFormat::eS8 >( p_pSrc, p_pDst, p_ePixelFmtDst );
		}
		static inline void convert( uint8_t const *& p_pSrcBuffer, uint32_t p_uiSrcSize, PixelFormat p_eDstFormat, uint8_t *& p_pDstBuffer, uint32_t p_uiDstSize )
		{
			detail::DynamicStencilBufferConversion< PixelFormat::eS8 >( p_pSrcBuffer, p_uiSrcSize, p_eDstFormat, p_pDstBuffer, p_uiDstSize );
		}
		template< PixelFormat PF > static inline void convert( uint8_t const *& p_pSrc, uint8_t *& p_pDst )
		{
			detail::pixel_converter< PixelFormat::eS8, PF >()( p_pSrc, p_pDst );
		}
	};
}