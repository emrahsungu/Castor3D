/*
See LICENSE file in root folder
*/
#ifndef ___C3D_ShaderUbosModule_H___
#define ___C3D_ShaderUbosModule_H___

#include "Castor3D/Shader/ShaderModule.hpp"
#include "Castor3D/Shader/Shaders/SdwModule.hpp"

#include <CastorUtils/Math/SquareMatrix.hpp>

namespace castor3d
{
	/**@name Shader */
	//@{
	/**@name UBO */
	//@{

	/**
	*\~english
	*\brief
	*	Billboards list UBO.
	*\~french
	*\brief
	*	UBO pour les billboards.
	*/
	class BillboardUbo;
	/**
	*\~english
	*\brief
	*	Billboards list data.
	*\~french
	*\brief
	*	Données pour les billboards.
	*/
	struct BillboardUboConfiguration
	{
		castor::Point2f dimensions;
	};
	/**
	*\~english
	*\brief
	*	Debug switchs.
	*\~french
	*\brief
	*	Les switchs de debug.
	*/
	struct DebugConfig;
	/**
	*\~english
	*\brief
	*	Geometry pass UBO.
	*\~french
	*\brief
	*	UBO de la Geometry pass.
	*/
	class GpInfoUbo;
	/**
	*\~english
	*\brief
	*	Geometry pass data.
	*\~french
	*\brief
	*	Données de la Geometry pass.
	*/
	struct GpInfoUboConfiguration
	{
		castor::Matrix4x4f invViewProj;
		castor::Matrix4x4f invView;
		castor::Matrix4x4f invProj;
		castor::Matrix4x4f gView;
		castor::Matrix4x4f gProj;
		castor::Point2f renderSize;
	};
	/**
	*\~english
	*\brief
	*	Light propagation UBO.
	*\~french
	*\brief
	*	UBO de light propagation.
	*/
	class LpvConfigUbo;
	/**
	*\~english
	*\brief
	*	Light propagation data.
	*\~french
	*\brief
	*	Données de light propagation.
	*/
	struct LpvConfigUboConfiguration
	{
		castor::Matrix4x4f lightView;
		castor::Point4f minVolumeCorner;
		castor::Point4f gridSizes;
		castor::Point4f config;
	};
	/**
	*\~english
	*\brief
	*	Light propagation UBO.
	*\~french
	*\brief
	*	UBO de light propagation.
	*/
	class LayeredLpvConfigUbo;
	/**
	*\~english
	*\brief
	*	Light propagation data.
	*\~french
	*\brief
	*	Données de light propagation.
	*/
	struct LayeredLpvConfigUboConfiguration
	{
		std::array< castor::Point4f, shader::DirectionalMaxCascadesCount > allMinVolumeCorners;
		castor::Point4f allCellSizes;
		castor::Point4ui gridSize;
		castor::Point4f config;
	};
	/**
	*\brief
	*	HDR configuration data UBO.
	*\~french
	*\brief
	*	UBO des données de configuration HDR.
	*/
	class HdrConfigUbo;
	/**
	*\~english
	*\brief
	*	Global matrices UBO.
	*\~french
	*\brief
	*	UBO des matrices globales.
	*/
	class MatrixUbo;
	/**
	*\~english
	*\brief
	*	Global matrices data.
	*\~french
	*\brief
	*	Données des matrices globales.
	*/
	struct MatrixUboConfiguration
	{
		castor::Matrix4x4f projection;
		castor::Matrix4x4f invProjection;
		castor::Matrix4x4f curView;
		castor::Matrix4x4f prvView;
		castor::Matrix4x4f curViewProj;
		castor::Matrix4x4f prvViewProj;
		castor::Point2f jitter;
	};
	/**
	*\~english
	*\brief
	*	Model matrices UBO.
	*\~french
	*\brief
	*	UBO des matrices modèle.
	*/
	class ModelMatrixUbo;
	/**
	*\~english
	*\brief
	*	Model matrices data.
	*\~french
	*\brief
	*	Données des matrices modèle.
	*/
	struct ModelMatrixUboConfiguration
	{
		castor::Matrix4x4f prvModel;
		castor::Matrix4x4f prvNormal;
		castor::Matrix4x4f curModel;
		castor::Matrix4x4f curNormal;
	};
	/**
	*\~english
	*\brief
	*	Model data UBO.
	*\~french
	*\brief
	*	UBO des données modèle.
	*/
	class ModelUbo;
	/**
	*\~english
	*\brief
	*	Model data.
	*\~french
	*\brief
	*	Données modèle.
	*/
	struct ModelUboConfiguration
	{
		int32_t shadowReceiver;
		int32_t materialIndex;
		int32_t environmentIndex;
	};
	/**
	*\~english
	*\brief
	*	Morphing data (time) UBO.
	*\~french
	*\brief
	*	UBO des données de morphing (temps).
	*/
	class MorphingUbo;
	/**
	*\~english
	*\brief
	*	Morphing data.
	*\~french
	*\brief
	*	Données de morphing (temps).
	*/
	struct MorphingUboConfiguration
	{
		float time;
	};
	/**
	*\~english
	*\brief
	*	Overlay data UBO.
	*\~french
	*\brief
	*	UBO des données d'overlays.
	*/
	class OverlayUbo;
	/**
	*\~english
	*\brief
	*	Overlay data.
	*\~french
	*\brief
	*	Données d'overlays.
	*/
	struct OverlayUboConfiguration
	{
		castor::Point4f positionRatio;
		castor::Point4i renderSizeIndex;
	};
	/**
	*\~english
	*\brief
	*	Picking data UBO.
	*\~french
	*\brief
	*	UBO des données de picking.
	*/
	class PickingUbo;
	/**
	*\~english
	*\brief
	*	Picking data.
	*\~french
	*\brief
	*	Données de picking.
	*/
	struct PickingUboConfiguration
	{
		uint32_t drawIndex;
		uint32_t nodeIndex;
	};
	/**
	*\~english
	*\brief
	*	Reflective Shadow Maps based GI UBO
	*\~french
	*\brief
	*	UBO du GI basé sur les Reflective Shadow Maps.
	*/
	class RsmConfigUbo;
	/**
	*\~english
	*\brief
	*	Reflective Shadow Maps based GI UBO configuration.
	*\~french
	*\brief
	*	Configuration de l'UBO du GI basé sur les Reflective Shadow Maps.
	*/
	struct RsmUboConfiguration
	{
		float intensity;
		float maxRadius;
		uint32_t sampleCount;
		uint32_t index;
	};
	/**
	*\~english
	*\brief
	*	Scene data UBO.
	*\~french
	*\brief
	*	UBO des données de scène.
	*/
	class SceneUbo;
	/**
	*\~english
	*\brief
	*	Scene data.
	*\~french
	*\brief
	*	données de scène.
	*/
	struct SceneUboConfiguration
	{
		castor::Point4f ambientLight;
		castor::Point4f backgroundColour;
		castor::Point4f lightsCount;
		castor::Point4f cameraPos;
		castor::Point4f clipInfo;
		castor::Point4f fogInfo;
	};
	/**
	*\~english
	*\brief
	*	Shadow map data UBO.
	*\~french
	*\brief
	*	UBO des données de shadow map.
	*/
	class ShadowMapUbo;
	/**
	*\~english
	*\brief
	*	Shadow map data.
	*\~french
	*\brief
	*	Données de shadow map.
	*/
	struct ShadowMapUboConfiguration
	{
		castor::Matrix4x4f lightProjection;
		castor::Matrix4x4f lightView;
		castor::Point4f lightPosFarPlane;
		uint32_t lightIndex;
	};
	/**
	*\~english
	*\brief
	*	Skinning data UBO.
	*\~french
	*\brief
	*	UBO des données de skinning.
	*/
	class SkinningUbo;
	/**
	*\~english
	*\brief
	*	Skinning data.
	*\~french
	*\brief
	*	Données de skinning.
	*/
	struct SkinningUboConfiguration
	{
		castor::Matrix4x4f bonesMatrix[400];
	};
	/**
	*\~english
	*\brief
	*	Texture configuration data UBO.
	*\~french
	*\brief
	*	UBO des données de configuration de texture.
	*/
	class TexturesUbo;
	/**
	*\~english
	*\brief
	*	Texture configuration data.
	*\~french
	*\brief
	*	Données de configuration de texture.
	*/
	struct TexturesUboConfiguration
	{
		std::array< castor::Point4ui, 3u > indices;
	};

	//@}
	//@}
}

#endif
