/*
See LICENSE file in root folder
*/
#ifndef ___C3D_BillboardRenderNode_H___
#define ___C3D_BillboardRenderNode_H___

#include "Castor3D/Render/Node/ObjectRenderNode.hpp"
#include "Castor3D/Shader/Ubos/BillboardUbo.hpp"

namespace castor3d
{
	struct BillboardRenderNode
		: public BillboardListRenderNode
	{
		C3D_API BillboardRenderNode( RenderPipeline & pipeline
			, PassRenderNode && passNode
			, UniformBufferOffset< ModelMatrixUbo::Configuration > modelMatrixBuffer
			, UniformBufferOffset< ModelUbo::Configuration > modelBuffer
			, UniformBufferOffset< PickingUbo::Configuration > pickingBuffer
			, UniformBufferOffset< BillboardUbo::Configuration > billboardBuffer
			, UniformBufferOffset< TexturesUbo::Configuration > texturesBuffer
			, GeometryBuffers const & buffers
			, SceneNode & sceneNode
			, BillboardBase & data );

		//!\~english	The billboard UBO.
		//!\~french		L'UBO de billboard.
		UniformBufferOffset< BillboardUbo::Configuration > billboardUbo;
	};
}

#endif
