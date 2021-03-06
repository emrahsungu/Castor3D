/*
See LICENSE file in root folder
*/
#ifndef ___C3D_ReflectiveShadowMapGIModule_H___
#define ___C3D_ReflectiveShadowMapGIModule_H___

#include "Castor3D/Render/Technique/Opaque/OpaqueModule.hpp"

#include "Castor3D/Scene/Light/LightModule.hpp"

namespace castor3d
{
	/**@name Render */
	//@{
	/**@name Technique */
	//@{
	/**@name Opaque */
	//@{
	/**@name RSM GI */
	//@{

	/**
	*\~english
	*\brief
	*	Reflective Shadow Map shader helpers.
	*\~french
	*\brief
	*	classe d'aide pour les shaders de Reflective Shadow Map.
	*/
	class ReflectiveShadowMapping;
	/**
	*\~english
	*\brief
	*	Reflective Shadow Maps configuration values.
	*\~french
	*\brief
	*	Valeurs de configuration des Reflective Shadow Maps.
	*/
	struct RsmConfig;
	/**
	*\~english
	*\brief
	*	SSGI pass based on Reflective Shadow Maps.
	*\~french
	*\brief
	*	Passe de SSGI basée sur les Reflective Shadow Maps.
	*/
	class RsmGIPass;
	/**
	*\~english
	*\brief
	*	RSM GI interpolation pass, used to optimise the algorithm.
	*\~french
	*\brief
	*	Passe d'interpolation du RSM GI, pour optimiser l'algorithme.
	*/
	class RsmInterpolatePass;
	/**
	*\~english
	*\brief
	*	Base class for all light passes with reflective shadow maps.
	*\~french
	*\brief
	*	Classe de base pour toutes les passes d'éclairage avec des reflective shadow maps.
	*/
	template< LightType LtType >
	class LightPassReflectiveShadow;

	//@}
	//@}
	//@}
	//@}
}

#endif
