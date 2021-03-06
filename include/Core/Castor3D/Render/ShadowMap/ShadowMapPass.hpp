/*
See LICENSE file in root folder
*/
#ifndef ___C3D_SHADOW_MAP_PASS_H___
#define ___C3D_SHADOW_MAP_PASS_H___

#include "ShadowMapModule.hpp"

#include "Castor3D/Material/Texture/TextureUnit.hpp"
#include "Castor3D/Render/RenderPass.hpp"
#include "Castor3D/Render/Viewport.hpp"
#include "Castor3D/Render/ShadowMap/ShadowMap.hpp"
#include "Castor3D/Scene/Camera.hpp"
#include "Castor3D/Scene/Geometry.hpp"
#include "Castor3D/Shader/Ubos/ShadowMapUbo.hpp"

#include <ashespp/Pipeline/PipelineVertexInputStateCreateInfo.hpp>

namespace castor3d
{
	class ShadowMapPass
		: public RenderPass
	{
	public:
		/**
		 *\~english
		 *\brief		Constructor.
		 *\param[in]	name		The pass name.
		 *\param[in]	engine		The engine.
		 *\param[in]	matrixUbo	The scene matrices UBO.
		 *\param[in]	culler		The culler for this pass.
		 *\param[in]	shadowMap	The parent shadow map.
		 *\~french
		 *\brief		Constructeur.
		 *\param[in]	name		Le nom de la passe.
		 *\param[in]	engine		Le moteur.
		 *\param[in]	matrixUbo	L'UBO de matrices de la scène.
		 *\param[in]	culler		Le culler pour cette passe.
		 *\param[in]	shadowMap	La shadow map parente.
		 */
		C3D_API ShadowMapPass( castor::String name
			, Engine & engine
			, MatrixUbo & matrixUbo
			, SceneCuller & culler
			, ShadowMap const & shadowMap );
		/**
		 *\~english
		 *\brief		Destructor.
		 *\~french
		 *\brief		Destructeur.
		 */
		C3D_API ~ShadowMapPass();
		/**
		 *\~english
		 *\brief		Updates the render pass.
		 *\remarks		Gather the render queues, for further update.
		 *\param[out]	queues	Receives the render queues needed for the rendering of the frame.
		 *\param[out]	light	The light source.
		 *\param[out]	index	The pass index.
		 *\return		\p true if the pass has changed.
		 *\~french
		 *\brief		Met à jour la passe de rendu.
		 *\remarks		Récupère les files de rendu, pour mise à jour ultérieure.
		 *\param[out]	queues	Reçoit les files de rendu nécessaires pour le dessin de la frame.
		 *\param[out]	light	La source lumineuse.
		 *\param[out]	index	L'indice de la passe.
		 *\return		\p true si la passe a changé.
		 */
		C3D_API virtual bool update( RenderQueueArray & queues
			, Light & light
			, uint32_t index ) = 0;
		/**
		 *\~english
		 *\brief		Updates device dependent data.
		 *\param[in]	index	The render index.
		 *\~french
		 *\brief		Met à jour les données dépendantes du device.
		 *\param[in]	index	L'indice du rendu.
		 */
		C3D_API virtual void updateDeviceDependent( uint32_t index = 0 ) = 0;

		inline RenderPassTimer & getTimer()
		{
			return *m_timer;
		}

		inline bool isUpToDate()const
		{
			return !m_outOfDate;
		}

		inline void setUpToDate()
		{
			m_outOfDate = false;
		}

		C3D_API TextureFlags getTexturesMask()const override
		{
			return ShadowMap::textureFlags;
		}

	protected:
		/**
		 *\~english
		 *\brief		Updates the given nodes.
		 *\param		nodes	The nodes.
		 *\~french
		 *\brief		Met à jour les noeuds donnés.
		 *\param		nodes	Les noeuds.
		 */
		void doUpdateNodes( SceneCulledRenderNodes & nodes );
		/**
		 *\copydoc		castor3d::RenderPass::doCreateTextureBindings
		 */
		ashes::VkDescriptorSetLayoutBindingArray doCreateTextureBindings( PipelineFlags const & flags )const override;

	private:
		/**
		 *\copydoc		castor3d::RenderPass::doFillTextureDescriptor
		 */
		void doFillTextureDescriptor( ashes::DescriptorSetLayout const & layout
			, uint32_t & index
			, BillboardListRenderNode & nodes
			, ShadowMapLightTypeArray const & shadowMaps )override;
		/**
		 *\copydoc		castor3d::RenderPass::doFillTextureDescriptor
		 */
		void doFillTextureDescriptor( ashes::DescriptorSetLayout const & layout
			, uint32_t & index
			, SubmeshRenderNode & nodes
			, ShadowMapLightTypeArray const & shadowMaps )override;
		/**
		 *\copydoc		castor3d::RenderPass::getGeometryShaderSource
		 */
		ShaderPtr doGetGeometryShaderSource( PipelineFlags const & flags )const override;
		/**
		 *\copydoc		castor3d::RenderPass::getGeometryShaderSource
		 */
		void doUpdatePipeline( RenderPipeline & pipeline )const override;

	protected:
		ShadowMap const & m_shadowMap;
		mutable bool m_initialised{ false };
		bool m_outOfDate{ true };
		ShadowMapUbo m_shadowMapUbo;
	};
}

#endif
