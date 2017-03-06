#include "ReinhardToneMapping.hpp"

#include <Log/Logger.hpp>

#include <Engine.hpp>
#include <Cache/TargetCache.hpp>

#include <Miscellaneous/Version.hpp>

using namespace Reinhard;

#ifndef CASTOR_PLATFORM_WINDOWS
#	define C3D_ReinhardToneMapping_API
#else
#	ifdef ReinhardToneMapping_EXPORTS
#		define C3D_ReinhardToneMapping_API __declspec( dllexport )
#	else
#		define C3D_ReinhardToneMapping_API __declspec( dllimport )
#	endif
#endif

C3D_ReinhardToneMapping_API void GetRequiredVersion( Castor3D::Version & p_version )
{
	p_version = Castor3D::Version();
}

C3D_ReinhardToneMapping_API Castor3D::PluginType GetType()
{
	return Castor3D::PluginType::eToneMapping;
}

C3D_ReinhardToneMapping_API Castor::String GetName()
{
	return cuT( "Reinhard Tone Mapping" );
}

C3D_ReinhardToneMapping_API void OnLoad( Castor3D::Engine * p_engine )
{
	p_engine->GetRenderTargetCache().GetToneMappingFactory().Register( ToneMapping::Name, &ToneMapping::Create );
}

C3D_ReinhardToneMapping_API void OnUnload( Castor3D::Engine * p_engine )
{
	p_engine->GetRenderTargetCache().GetToneMappingFactory().Unregister( ToneMapping::Name );
}