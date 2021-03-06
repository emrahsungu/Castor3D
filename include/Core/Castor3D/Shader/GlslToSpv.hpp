/*
This file belongs to Ashes.
See LICENSE file in root folder.
*/
#pragma once

#include "ShaderModule.hpp"
#include "Castor3D/Render/RenderModule.hpp"

namespace castor3d
{
	/**
	*\~french
	*\brief
	*	Initialise les globales de glslang.
	*\~english
	*\brief
	*	Initialises glslang globals.
	*/
	C3D_API void initialiseGlslang();
	/**
	*\~french
	*\brief
	*	Nettoie les globales de glslang.
	*\~english
	*\brief
	*	Cleans up glslang globals.
	*/
	C3D_API void cleanupGlslang();
	/**
	*\~french
	*\brief
	*	Transpile un shader GLSL en SPIR-V.
	*\~english
	*\brief
	*	Transpiles a GLSL shader to SPIR-V.
	*/
	C3D_API UInt32Array compileGlslToSpv( RenderDevice const & device
		, VkShaderStageFlagBits stage
		, std::string const & shader );
}
