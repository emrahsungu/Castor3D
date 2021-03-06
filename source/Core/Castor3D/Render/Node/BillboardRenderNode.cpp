#include "Castor3D/Render/Node/BillboardRenderNode.hpp"

#include "Castor3D/Render/RenderPipeline.hpp"
#include "Castor3D/Scene/SceneNode.hpp"
#include "Castor3D/Scene/BillboardList.hpp"
#include "Castor3D/Shader/Program.hpp"

using namespace castor;

namespace castor3d
{
	BillboardRenderNode::BillboardRenderNode( RenderPipeline & pipeline
		, PassRenderNode && passNode
		, UniformBufferOffset< ModelMatrixUbo::Configuration > modelMatrixBuffer
		, UniformBufferOffset< ModelUbo::Configuration > modelBuffer
		, UniformBufferOffset< PickingUbo::Configuration > pickingBuffer
		, UniformBufferOffset< BillboardUbo::Configuration > billboardBuffer
		, UniformBufferOffset< TexturesUbo::Configuration > texturesBuffer
		, GeometryBuffers const & buffers
		, SceneNode & sceneNode
		, BillboardBase & data )
		: BillboardListRenderNode{ pipeline
			, std::move( passNode )
			, modelMatrixBuffer
			, modelBuffer
			, pickingBuffer
			, texturesBuffer
			, buffers
			, sceneNode
			, data
			, data }
		, billboardUbo{ billboardBuffer }
	{
	}
}
