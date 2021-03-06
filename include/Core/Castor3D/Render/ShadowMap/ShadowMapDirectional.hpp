/*
See LICENSE file in root folder
*/
#ifndef ___C3D_ShadowMapDirectional_H___
#define ___C3D_ShadowMapDirectional_H___

#include "ShadowMapModule.hpp"

#include "Castor3D/Render/Passes/GaussianBlur.hpp"
#include "Castor3D/Render/ShadowMap/ShadowMap.hpp"

#include <ashespp/Image/ImageView.hpp>

namespace castor3d
{
	class ShadowMapDirectional
		: public ShadowMap
	{
	public:
		/**
		 *\~english
		 *\brief		Constructor.
		 *\param[in]	engine		The engine.
		 *\param[in]	scene		The scene.
		 *\~french
		 *\brief		Constructeur.
		 *\param[in]	engine		Le moteur.
		 *\param[in]	scene		La scène.
		 */
		ShadowMapDirectional( Engine & engine
			, Scene & scene );
		/**
		 *\~english
		 *\brief		Destructor.
		 *\~french
		 *\brief		Destructeur.
		 */
		~ShadowMapDirectional();
		/**
		 *\copydoc		castor3d::ShadowMap::update
		 */
		void update( Camera const & camera
			, RenderQueueArray & queues
			, Light & light
			, uint32_t index )override;
		/**
		 *\copydoc		castor3d::ShadowMap::updateDeviceDependent
		 */
		void updateDeviceDependent( uint32_t index )override;

	private:
		void doInitialiseFramebuffers();
		/**
		 *\copydoc		castor3d::ShadowMap::doInitialise
		 */
		void doInitialise()override;
		/**
		 *\copydoc		castor3d::ShadowMap::doCleanup
		 */
		void doCleanup()override;
		/**
		 *\copydoc		castor3d::ShadowMap::doRender
		 */
		ashes::Semaphore const & doRender( ashes::Semaphore const & toWait
			, uint32_t index )override;
		/**
		 *\copydoc		castor3d::ShadowMap::isUpToDate
		 */
		bool isUpToDate( uint32_t index )const override;

	public:
		static VkFormat constexpr VarianceFormat = VK_FORMAT_R32G32_SFLOAT;
		static VkFormat constexpr LinearDepthFormat = VK_FORMAT_R32_SFLOAT;
		static VkFormat RawDepthFormat;

	private:
		struct FrameBuffer
		{
			ashes::FrameBufferPtr frameBuffer;
			ashes::ImageView depthView;
			ashes::ImageView linearView;
			ashes::ImageView varianceView;
			ashes::ImageView positionView;
			ashes::ImageView fluxView;
			std::unique_ptr< GaussianBlur > blur;
			CommandsSemaphore blurCommands{ nullptr, nullptr };
		};
		ashes::CommandBufferPtr m_commandBuffer;
		CameraSPtr m_camera;
		std::vector< FrameBuffer > m_frameBuffers;
		ShadowType m_shadowType{ ShadowType::eRaw };
		uint32_t m_cascades;
	};
}

#endif
