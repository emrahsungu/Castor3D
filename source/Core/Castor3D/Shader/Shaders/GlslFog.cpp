#include "Castor3D/Shader/Shaders/GlslFog.hpp"

#include <ShaderWriter/Source.hpp>

using namespace castor;
using namespace sdw;

namespace castor3d
{
	namespace shader
	{
		Fog::Fog( FogType flags, ShaderWriter & writer )
		{
			if ( flags != FogType::eDisabled )
			{
				m_fog = writer.implementFunction< Vec4 >( "applyFog"
					, [&writer, flags]( Vec4 const & bgColour
						, Vec4 const & colour
						, Float const & dist
						, Float const & y
						, Vec4 const & fogInfo
						, Vec4 const & cameraPosition )
					{
						auto z = writer.declLocale( "z", dist / 100.0_f );
						auto density = writer.declLocale( "density", fogInfo.y() );

						if ( flags == FogType::eLinear )
						{
							// Linear
							auto fogFactor = writer.declLocale( "fogFactor"
								, ( 80.0_f - z ) / ( 80.0_f - 20.0_f ) );
							fogFactor = clamp( fogFactor, 0.0_f, 1.0_f );
							writer.returnStmt( vec4( mix( bgColour, colour, vec4( fogFactor ) ).rgb(), colour.a() ) );
						}
						else if ( flags == FogType::eExponential )
						{
							// Exponential
							auto fogFactor = writer.declLocale( "fogFactor"
								, exp( -density * z / 100.0_f ) );
							fogFactor = clamp( fogFactor, 0.0_f, 1.0_f );
							writer.returnStmt( vec4( mix( bgColour, colour, vec4( fogFactor ) ).rgb(), colour.a() ) );
						}
						else if ( flags == FogType::eSquaredExponential )
						{
							//Squared exponential
							auto fogFactor = writer.declLocale( "fogFactor"
								, exp( -density * density * z * z / 100.0_f ) );
							fogFactor = clamp( fogFactor, 0.0_f, 1.0_f );
							writer.returnStmt( vec4( mix( bgColour, colour, vec4( fogFactor ) ).rgb(), colour.a() ) );
						}
						else if ( flags == FogType( 4 ) )
						{
							// Coloured
							auto fogFactor = writer.declLocale( "fogFactor"
								, 0.0_f );
							fogFactor = clamp( fogFactor, 0.0_f, 1.0_f );
							writer.returnStmt( vec4( mix( bgColour, colour, vec4( fogFactor ) ).rgb(), colour.a() ) );
						}
						else if ( flags == FogType( 5 ) )
						{
							// Ground
							//my camera y is 10.0. you can change it or pass it as a uniform
							auto be = writer.declLocale( "be"
								, ( cameraPosition.y() - y ) * 0.004_f );// 0.004 is just a factor; change it if you want
							auto bi = writer.declLocale( "bi"
								, ( cameraPosition.y() - y ) * 0.001_f );// 0.001 is just a factor; change it if you want
							auto extinction = writer.declLocale( "ext"
								, exp( -z * be ) );
							auto inscattering = writer.declLocale( "insc"
								, exp( -z * bi ) );
							writer.returnStmt( sdw::fma( bgColour
								, vec4( 1.0_f - inscattering )
								, colour * extinction ) );
						}
					}
					, InVec4( writer, "bgColour" )
					, InVec4( writer, "colour" )
					, InFloat( writer, "dist" )
					, InFloat( writer, "y" )
					, InVec4( writer, "fogInfo" )
					, InVec4( writer, "cameraPosition" ) );
			}
		}

		Vec4 Fog::apply( Vec4 const & bgColour
			, Vec4 const & colour
			, Float const & dist
			, Float const & y
			, Vec4 const & fogInfo
			, Vec4 const & cameraPosition )
		{
			return m_fog( bgColour
				, colour
				, dist
				, y
				, fogInfo 
				, cameraPosition );
		}
}
}
