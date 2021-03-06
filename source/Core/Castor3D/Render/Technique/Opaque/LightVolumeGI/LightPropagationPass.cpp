#include "Castor3D/Render/Technique/Opaque/LightVolumeGI/LightPropagationPass.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Cache/LightCache.hpp"
#include "Castor3D/Cache/SamplerCache.hpp"
#include "Castor3D/Material/Texture/Sampler.hpp"
#include "Castor3D/Material/Texture/TextureLayout.hpp"
#include "Castor3D/Material/Texture/TextureUnit.hpp"
#include "Castor3D/Miscellaneous/PipelineVisitor.hpp"
#include "Castor3D/Render/RenderPass.hpp"
#include "Castor3D/Render/RenderSystem.hpp"
#include "Castor3D/Render/RenderPassTimer.hpp"
#include "Castor3D/Render/ShadowMap/ShadowMapResult.hpp"
#include "Castor3D/Scene/Camera.hpp"
#include "Castor3D/Scene/Scene.hpp"
#include "Castor3D/Scene/SceneNode.hpp"
#include "Castor3D/Scene/Light/Light.hpp"
#include "Castor3D/Shader/Program.hpp"
#include "Castor3D/Shader/Shaders/GlslLight.hpp"
#include "Castor3D/Shader/Shaders/GlslLighting.hpp"
#include "Castor3D/Shader/Shaders/GlslOutputComponents.hpp"
#include "Castor3D/Shader/Shaders/GlslPhongLighting.hpp"
#include "Castor3D/Shader/Shaders/GlslUtils.hpp"
#include "Castor3D/Shader/Ubos/GpInfoUbo.hpp"
#include "Castor3D/Shader/Ubos/LpvConfigUbo.hpp"

#include <CastorUtils/Graphics/Image.hpp>

#include <ashespp/Image/Image.hpp>
#include <ashespp/Image/ImageView.hpp>
#include <ashespp/RenderPass/RenderPass.hpp>
#include <ashespp/RenderPass/RenderPassCreateInfo.hpp>
#include <ashespp/RenderPass/SubpassDescription.hpp>

#include <ShaderWriter/Source.hpp>

#include <numeric>
#include <random>

using namespace castor;

namespace castor3d
{
	namespace
	{
		static constexpr uint32_t LIUboIdx = 0u;
		static constexpr uint32_t RLpvGridIdx = 1u;
		static constexpr uint32_t GLpvGridIdx = 2u;
		static constexpr uint32_t BLpvGridIdx = 3u;
		static constexpr uint32_t GpGridIdx = 4u;

		std::unique_ptr< ast::Shader > getVertexProgram()
		{
			using namespace sdw;
			VertexWriter writer;

			auto inPosition = writer.declInput< Vec3 >( "inPosition", 0u );

			auto outCellIndex = writer.declOutput< IVec3 >( "outCellIndex", 0u );
			auto out = writer.getOut();

			UBO_LPVCONFIG( writer, LIUboIdx, 0u );

			writer.implementFunction< Void >( "main"
				, [&]()
				{
					outCellIndex = ivec3( inPosition );
					auto screenPos = writer.declLocale( "screenPos"
						, ( inPosition.xy() + 0.5_f ) / c3d_gridSizes.xy() * 2.0_f - 1.0_f );
					out.vtx.position = vec4( screenPos, 0.0, 1.0 );
				} );
			return std::make_unique< ast::Shader >( std::move( writer.getShader() ) );
		}

		std::unique_ptr< ast::Shader > getGeometryProgram()
		{
			using namespace sdw;
			GeometryWriter writer;
			writer.inputLayout( ast::stmt::InputLayout::ePointList );
			writer.outputLayout( ast::stmt::OutputLayout::ePointList, 1u );

			auto inCellIndex = writer.declInputArray< IVec3 >( "inCellIndex", 0u, 1u );
			auto in = writer.getIn();

			auto outCellIndex = writer.declOutput< IVec3 >( "outCellIndex", 0u );
			auto out = writer.getOut();

			writer.implementFunction< Void >( "main"
				, [&]()
				{
					out.vtx.position = in.vtx[0].position;
					out.vtx.pointSize = 1.0f;
					out.layer = inCellIndex[0].z();

					outCellIndex = inCellIndex[0];

					EmitVertex( writer );
					EndPrimitive( writer );
				} );

			return std::make_unique< ast::Shader >( std::move( writer.getShader() ) );
		}

		ShaderPtr getPixelProgram( bool occlusion )
		{
			using namespace sdw;
			FragmentWriter writer;

			/*Spherical harmonics coefficients - precomputed*/
			auto SH_C0 = writer.declConstant( "SH_C0"
				, 0.282094792_f );// 1 / (2 * sqrt(pi))
			auto SH_C1 = writer.declConstant( "SH_C1"
				, 0.488602512_f ); // sqrt(3 / pi) / 2

			/*Cosine lobe coeff*/
			auto SH_cosLobe_C0 = writer.declConstant( "SH_cosLobe_C0"
				, 0.886226925_f );// sqrt(pi) / 2
			auto SH_cosLobe_C1 = writer.declConstant( "SH_cosLobe_C1"
				, 1.02332671_f ); // sqrt(pi / 3)

			auto directFaceSubtendedSolidAngle = writer.declConstant( "directFaceSubtendedSolidAngle"
				, 0.12753712_f ); // 0.4006696846f / Pi
			auto sideFaceSubtendedSolidAngle = writer.declConstant( "sideFaceSubtendedSolidAngle"
				, 0.13478556_f ); // 0.4234413544f / Pi
			auto propDirections = writer.declConstantArray( "propDirections"
				, std::vector< IVec3 >
				{
					//+Z
					ivec3( 0_i, 0, 1 ),
					//-Z
					ivec3( 0_i, 0, -1 ),
					//+X
					ivec3( 1_i, 0, 0 ),
					//-X
					ivec3( -1_i, 0, 0 ),
					//+Y
					ivec3( 0_i, 1, 0 ),
					//-Y
					ivec3( 0_i, -1, 0 ),
				} );

			//Sides of the cell - right, top, left, bottom
			auto cellSides = writer.declConstantArray( "cellSides"
				, std::vector< IVec2 >
				{
					ivec2( 1_i, 0 ),
					ivec2( 0_i, 1 ),
					ivec2( -1_i, 0 ),
					ivec2( 0_i, -1 ),
				} );

			UBO_LPVCONFIG( writer, LIUboIdx, 0u );
			auto c3d_lpvGridR = writer.declSampledImage< FImg3DRgba16 >( getTextureName( LpvTexture::eR, "Grid" ), RLpvGridIdx, 0u );
			auto c3d_lpvGridG = writer.declSampledImage< FImg3DRgba16 >( getTextureName( LpvTexture::eG, "Grid" ), GLpvGridIdx, 0u );
			auto c3d_lpvGridB = writer.declSampledImage< FImg3DRgba16 >( getTextureName( LpvTexture::eB, "Grid" ), BLpvGridIdx, 0u );
			auto c3d_geometryVolume = writer.declSampledImage< FImg3DRgba16 >( "c3d_geometryVolume", GpGridIdx, 0u, occlusion );

			uint32_t index = 0u;
			auto outLpvAccumulatorR = writer.declOutput< Vec4 >( "outLpvAccumulatorR", index++ );
			auto outLpvAccumulatorG = writer.declOutput< Vec4 >( "outLpvAccumulatorG", index++ );
			auto outLpvAccumulatorB = writer.declOutput< Vec4 >( "outLpvAccumulatorB", index++ );
			auto outLpvNextStepR = writer.declOutput< Vec4 >( "outLpvNextStepR", index++ );
			auto outLpvNextStepG = writer.declOutput< Vec4 >( "outLpvNextStepG", index++ );
			auto outLpvNextStepB = writer.declOutput< Vec4 >( "outLpvNextStepB", index++ );

			auto inCellIndex = writer.declInput< IVec3 >( "inCellIndex", 0u );
			auto in = writer.getIn();

			// no normalization
			auto evalSH_direct = writer.implementFunction< Vec4 >( "evalSH_direct"
				, [&]( Vec3 direction )
				{
					writer.returnStmt( vec4( SH_C0
						, -SH_C1 * direction.y()
						, SH_C1 * direction.z()
						, -SH_C1 * direction.x() ) );
				}
				, InVec3{ writer, "direction" } );

			// no normalization
			auto evalCosineLobeToDir_direct = writer.implementFunction< Vec4 >( "evalCosineLobeToDir_direct"
				, [&]( Vec3 direction )
				{
					writer.returnStmt( vec4( SH_cosLobe_C0
						, -SH_cosLobe_C1 * direction.y()
						, SH_cosLobe_C1 * direction.z()
						, -SH_cosLobe_C1 * direction.x() ) );
				}
				, InVec3{ writer, "direction" } );

			//Get side direction
			auto getEvalSideDirection = writer.implementFunction< Vec3 >( "getEvalSideDirection"
				, [&]( Int index
					, IVec3 orientation )
				{
					const float smallComponent = 0.4472135f; // 1 / sqrt(5)
					const float bigComponent = 0.894427f; // 2 / sqrt(5)

					auto tmp = writer.declLocale( "tmp"
						, vec3( writer.cast< Float >( cellSides[index].x() ) * smallComponent
							, writer.cast< Float >( cellSides[index].y() ) * smallComponent
							, bigComponent ) );
					writer.returnStmt( vec3( orientation ) * tmp );
				}
				, InInt{ writer, "index" }
				, InIVec3{ writer, "orientation" } );

			auto getReprojSideDirection = writer.implementFunction< Vec3 >( "getReprojSideDirection"
				, [&]( Int index
					, IVec3 orientation )
				{
					writer.returnStmt( vec3( orientation.x() * cellSides[index].x()
						, orientation.y() * cellSides[index].y()
						, 0 ) );
				}
				, InInt{ writer, "index" }
				, InIVec3{ writer, "orientation" } );

			float occlusionAmplifier = 1.0f;

			auto propagate = writer.implementFunction< Void >( "propagate"
				, [&]( Vec4 R
					, Vec4 G
					, Vec4 B )
				{
					FOR( writer, Int, neighbour, 0, neighbour < 6, neighbour++ )
					{
						auto RSHcoeffsNeighbour = writer.declLocale( "RSHcoeffsNeighbour"
							, vec4( 0.0_f ) );
						auto GSHcoeffsNeighbour = writer.declLocale( "GSHcoeffsNeighbour"
							, vec4( 0.0_f ) );
						auto BSHcoeffsNeighbour = writer.declLocale( "BSHcoeffsNeighbour"
							, vec4( 0.0_f ) );
						//Get main direction
						auto mainDirection = writer.declLocale( "mainDirection"
							, propDirections[neighbour] );
						//get neighbour cell indexindex
						auto neighbourGScellIndex = writer.declLocale( "neighbourGScellIndex"
							, inCellIndex - mainDirection );
						//Load sh coeffs
						RSHcoeffsNeighbour = texelFetch( c3d_lpvGridR, neighbourGScellIndex, 0_i );
						GSHcoeffsNeighbour = texelFetch( c3d_lpvGridG, neighbourGScellIndex, 0_i );
						BSHcoeffsNeighbour = texelFetch( c3d_lpvGridB, neighbourGScellIndex, 0_i );

						auto occlusionValue = writer.declLocale( "occlusionValue"
							, 1.0_f ); // no occlusion

						if ( occlusion )
						{
							auto occCoord = writer.declLocale( "occCoord"
								, ( vec3( neighbourGScellIndex.xyz() ) + 0.5_f * vec3( mainDirection ) ) / c3d_gridSizes.xyz() );
							auto occCoeffs = writer.declLocale( "occCoeffs"
								, texture( c3d_geometryVolume, occCoord ) );
							occlusionValue = 1.0 - clamp( occlusionAmplifier * dot( occCoeffs, evalSH_direct( vec3( -mainDirection ) ) ), 0.0_f, 1.0_f );
						}

						auto occludedDirectFaceContribution = writer.declLocale( "occludedDirectFaceContribution"
							, occlusionValue * directFaceSubtendedSolidAngle );

						auto mainDirectionCosineLobeSH = writer.declLocale( "mainDirectionCosineLobeSH"
							, evalCosineLobeToDir_direct( vec3( mainDirection ) ) );
						auto mainDirectionSH = writer.declLocale( "mainDirectionSH"
							, evalSH_direct( vec3( mainDirection ) ) );
						R += occludedDirectFaceContribution * max( 0.0_f, dot( RSHcoeffsNeighbour, mainDirectionSH ) ) * mainDirectionCosineLobeSH;
						G += occludedDirectFaceContribution * max( 0.0_f, dot( GSHcoeffsNeighbour, mainDirectionSH ) ) * mainDirectionCosineLobeSH;
						B += occludedDirectFaceContribution * max( 0.0_f, dot( BSHcoeffsNeighbour, mainDirectionSH ) ) * mainDirectionCosineLobeSH;

						//Now we have contribution for the neighbour's cell in the main direction -> need to do reprojection 
						//Reprojection will be made only onto 4 faces (acctually we need to take into account 5 faces but we already have the one in the main direction)
						FOR( writer, Int, face, 0, face < 4, face++ )
						{
							//Get the direction to the face
							auto evalDirection = writer.declLocale( "evalDirection"
								, getEvalSideDirection( face, mainDirection ) );
							//Reprojected direction
							auto reprojDirection = writer.declLocale( "reprojDirection"
								, getReprojSideDirection( face, mainDirection ) );

							if ( occlusion )
							{
								auto occCoord = writer.declLocale( "occCoord"
									, ( vec3( neighbourGScellIndex.xyz() ) + 0.5_f * evalDirection ) / c3d_gridSizes.xyz() );
								auto occCoeffs = writer.declLocale( "occCoeffs"
									, texture( c3d_geometryVolume, occCoord ) );
								occlusionValue = 1.0 - clamp( occlusionAmplifier * dot( occCoeffs, evalSH_direct( -evalDirection ) ), 0.0_f, 1.0_f );
							}

							auto occludedSideFaceContribution = writer.declLocale( "occludedSideFaceContribution"
								, occlusionValue * sideFaceSubtendedSolidAngle );

							//Get sh coeff
							auto reprojDirectionCosineLobeSH = writer.declLocale( "reprojDirectionCosineLobeSH"
								, evalCosineLobeToDir_direct( reprojDirection ) );
							auto evalDirectionSH = writer.declLocale( "evalDirectionSH"
								, evalSH_direct( evalDirection ) );

							R += occludedSideFaceContribution * max( 0.0_f, dot( RSHcoeffsNeighbour, evalDirectionSH ) ) * reprojDirectionCosineLobeSH;
							G += occludedSideFaceContribution * max( 0.0_f, dot( GSHcoeffsNeighbour, evalDirectionSH ) ) * reprojDirectionCosineLobeSH;
							B += occludedSideFaceContribution * max( 0.0_f, dot( BSHcoeffsNeighbour, evalDirectionSH ) ) * reprojDirectionCosineLobeSH;
						}
						ROF;
					}
					ROF;
				}
				, OutVec4{ writer, "R" }
				, OutVec4{ writer, "G" }
				, OutVec4{ writer, "B" } );

			writer.implementMain( [&]()
				{
					auto R = writer.declLocale( "R"
						, vec4( 0.0_f ) );
					auto G = writer.declLocale( "G"
						, vec4( 0.0_f ) );
					auto B = writer.declLocale( "B"
						, vec4( 0.0_f ) );

					propagate( R, G, B );

					outLpvAccumulatorR = R;
					outLpvAccumulatorG = G;
					outLpvAccumulatorB = B;

					outLpvNextStepR = R;
					outLpvNextStepG = G;
					outLpvNextStepB = B;
				} );

			return std::make_unique< ast::Shader >( std::move( writer.getShader() ) );
		}

		ashes::RenderPassPtr doCreateRenderPass( castor::String const & name
			, RenderDevice const & device
			, VkFormat format )
		{
			ashes::VkAttachmentDescriptionArray attaches
			{
				{
					0u,
					format,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
				{
					0u,
					format,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
				{
					0u,
					format,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
				{
					0u,
					format,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
				{
					0u,
					format,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
				{
					0u,
					format,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
			};
			ashes::SubpassDescriptionArray subpasses;
			subpasses.emplace_back( ashes::SubpassDescription
				{
					0u,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					{},
					{
						{ 0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
						{ 1u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
						{ 2u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
						{ 3u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
						{ 4u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
						{ 5u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
					},
					{},
					ashes::nullopt,
					{},
				} );
			ashes::VkSubpassDependencyArray dependencies
			{
				{
					VK_SUBPASS_EXTERNAL,
					0u,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
				},
				{
					0u,
					VK_SUBPASS_EXTERNAL,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
				},
			};
			ashes::RenderPassCreateInfo createInfo
			{
				0u,
				std::move( attaches ),
				std::move( subpasses ),
				std::move( dependencies ),
			};
			auto result = device->createRenderPass( name
				, std::move( createInfo ) );
			return result;
		}

		ashes::VertexBufferPtr< castor::Point3f > doCreateVertexBuffer( castor::String const & name
			, Engine & engine
			, uint32_t width )
		{
			auto size = width * width * width;
			auto & device = getCurrentRenderDevice( engine );

			auto result = makeVertexBuffer< castor::Point3f >( device
				, size
				, 0u
				, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				, name );
			NonTexturedQuad::Vertex vtx;

			if ( auto buffer = result->lock( 0u, size, 0u ) )
			{
				for ( uint32_t d = 0; d < width; d++ )
				{
					for ( uint32_t c = 0; c < width; c++ )
					{
						for ( uint32_t r = 0; r < width; r++ )
						{
							*buffer = castor::Point3f{ float( r ), float( c ), float( d ) };
							++buffer;
						}
					}
				}

				result->flush( 0u, size );
				result->unlock();
			}

			return result;
		}

		ashes::DescriptorSetLayoutPtr doCreateDescriptorLayout( castor::String const & name
			, Engine & engine
			, bool occlusion )
		{
			ashes::VkDescriptorSetLayoutBindingArray bindings
			{
				makeDescriptorSetLayoutBinding( LIUboIdx
					, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
					, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT ),
				makeDescriptorSetLayoutBinding( RLpvGridIdx
					, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
					, VK_SHADER_STAGE_FRAGMENT_BIT ),
				makeDescriptorSetLayoutBinding( GLpvGridIdx
					, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
					, VK_SHADER_STAGE_FRAGMENT_BIT ),
				makeDescriptorSetLayoutBinding( BLpvGridIdx
					, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
					, VK_SHADER_STAGE_FRAGMENT_BIT ),
			};

			if ( occlusion )
			{
				bindings.push_back( makeDescriptorSetLayoutBinding( GpGridIdx
					, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
					, VK_SHADER_STAGE_FRAGMENT_BIT ) );
			}

			return getCurrentRenderDevice( engine )->createDescriptorSetLayout( name
				, std::move( bindings ) );
		}

		ashes::DescriptorSetPtr doCreateDescriptorSet( castor::String const & name
			, ashes::DescriptorSetPool & descriptorSetPool
			, TextureUnit const * geometryInjectionResult
			, LightVolumePassResult const & lightInjectionResult
			, UniformBuffer< LpvConfigUboConfiguration > const & ubo )
		{
			auto & descriptorSetLayout = descriptorSetPool.getLayout();
			auto result = descriptorSetPool.createDescriptorSet( name );
			result->createSizedBinding( descriptorSetLayout.getBinding( LIUboIdx )
				, ubo );
			result->createBinding( descriptorSetLayout.getBinding( RLpvGridIdx )
				, lightInjectionResult[LpvTexture::eR].getTexture()->getDefaultView().getSampledView()
				, lightInjectionResult[LpvTexture::eR].getSampler()->getSampler() );
			result->createBinding( descriptorSetLayout.getBinding( GLpvGridIdx )
				, lightInjectionResult[LpvTexture::eG].getTexture()->getDefaultView().getSampledView()
				, lightInjectionResult[LpvTexture::eG].getSampler()->getSampler() );
			result->createBinding( descriptorSetLayout.getBinding( BLpvGridIdx )
				, lightInjectionResult[LpvTexture::eB].getTexture()->getDefaultView().getSampledView()
				, lightInjectionResult[LpvTexture::eB].getSampler()->getSampler() );

			if ( geometryInjectionResult )
			{
				result->createBinding( descriptorSetLayout.getBinding( GpGridIdx )
					, geometryInjectionResult->getTexture()->getDefaultView().getSampledView()
					, geometryInjectionResult->getSampler()->getSampler() );
			}

			result->update();
			return result;
		}

		ashes::GraphicsPipelinePtr doCreatePipeline( castor::String const & name
			, Engine & engine
			, ashes::PipelineLayout const & pipelineLayout
			, ashes::RenderPass const & renderPass
			, ShaderModule const & vertexShader
			, ShaderModule const & geometryShader
			, ShaderModule const & pixelShader
			, uint32_t lpvSize )
		{
			ashes::VkVertexInputAttributeDescriptionArray attributes
			{
				{ 0u, 0u, VK_FORMAT_R32G32B32_SFLOAT, 0u },
			};
			ashes::PipelineVertexInputStateCreateInfo vertexState
			{
				0u,
				ashes::VkVertexInputBindingDescriptionArray
				{
					{ 0u, sizeof( castor::Point3f ), VK_VERTEX_INPUT_RATE_VERTEX },
				},
				std::move( attributes ),
			};
			VkViewport viewport{ 0.0f, 0.0f, float( lpvSize ), float( lpvSize ) };
			VkRect2D scissor{ 0, 0, lpvSize, lpvSize };
			ashes::PipelineViewportStateCreateInfo vpState{ 0u
				, 1u
				, ashes::VkViewportArray{ viewport }
				, 1u
				, ashes::VkScissorArray{ scissor } };
			auto & device = getCurrentRenderDevice( engine );
			ashes::PipelineShaderStageCreateInfoArray shaderStages;
			shaderStages.push_back( makeShaderState( device, vertexShader ) );
			shaderStages.push_back( makeShaderState( device, geometryShader ) );
			shaderStages.push_back( makeShaderState( device, pixelShader ) );
			return device->createPipeline( name
				, ashes::GraphicsPipelineCreateInfo{ 0u
					, shaderStages
					, std::move( vertexState )
					, ashes::PipelineInputAssemblyStateCreateInfo{ 0u, VK_PRIMITIVE_TOPOLOGY_POINT_LIST }
					, ashes::nullopt
					, std::move( vpState )
					, ashes::PipelineRasterizationStateCreateInfo{ 0u, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE }
					, ashes::PipelineMultisampleStateCreateInfo{}
					, ashes::PipelineDepthStencilStateCreateInfo{ 0u, VK_FALSE, VK_FALSE }
					, RenderPass::createBlendState( BlendMode::eNoBlend, BlendMode::eNoBlend, 6u )
					, ashes::nullopt
					, pipelineLayout
					, renderPass } );
		}
	}

	//*********************************************************************************************

	LightPropagationPass::LightPropagationPass( Engine & engine
		, castor::String const & suffix
		, uint32_t gridSize
		, TextureUnit const * geometry
		, LightVolumePassResult const & injection
		, LightVolumePassResult const & accumulation
		, LightVolumePassResult const & propagate
		, LpvConfigUbo const & lpvConfigUbo )
		: Named{ "LightPropagation" + suffix }
		, m_engine{ engine }
		, m_lpvConfigUbo{ lpvConfigUbo }
		, m_accumulation{ accumulation }
		, m_propagate{ propagate }
		, m_timer{ std::make_shared< RenderPassTimer >( engine, cuT( "Light Propagation Volumes" ), "Light Propagation " + suffix ) }
		, m_count{ gridSize * gridSize * gridSize }
		, m_vertexBuffer{ doCreateVertexBuffer( getName()
			, engine
			, gridSize ) }
		, m_descriptorSetLayout{ doCreateDescriptorLayout( getName()
			, engine
			, geometry != nullptr ) }
		, m_pipelineLayout{ getCurrentRenderDevice( m_engine )->createPipelineLayout( getName(), *m_descriptorSetLayout ) }
		, m_descriptorSetPool{ m_descriptorSetLayout->createPool( getName(), 1u ) }
		, m_descriptorSet{ doCreateDescriptorSet( getName()
			, *m_descriptorSetPool
			, geometry
			, injection
			, m_lpvConfigUbo.getUbo() ) }
		, m_vertexShader{ VK_SHADER_STAGE_VERTEX_BIT
			, getName()
			, getVertexProgram() }
		, m_geometryShader{ VK_SHADER_STAGE_GEOMETRY_BIT
			, getName()
			, getGeometryProgram() }
		, m_pixelShader{ VK_SHADER_STAGE_FRAGMENT_BIT
			, getName()
			, getPixelProgram( geometry != nullptr ) }
		, m_renderPass{ doCreateRenderPass( getName()
			, getCurrentRenderDevice( m_engine )
			, getFormat( LpvTexture::eR ) ) }
		, m_pipeline{ doCreatePipeline( getName()
			, m_engine
			, *m_pipelineLayout
			, *m_renderPass
			, m_vertexShader
			, m_geometryShader
			, m_pixelShader
			, gridSize ) }
		, m_frameBuffer{ m_renderPass->createFrameBuffer( getName()
			, VkExtent2D{ gridSize, gridSize }
			, { m_accumulation[LpvTexture::eR].getTexture()->getDefaultView().getTargetView()
				, m_accumulation[LpvTexture::eG].getTexture()->getDefaultView().getTargetView()
				, m_accumulation[LpvTexture::eB].getTexture()->getDefaultView().getTargetView()
				, m_propagate[LpvTexture::eR].getTexture()->getDefaultView().getTargetView()
				, m_propagate[LpvTexture::eG].getTexture()->getDefaultView().getTargetView()
				, m_propagate[LpvTexture::eB].getTexture()->getDefaultView().getTargetView() }
			, gridSize ) }
		, m_commands{ getCommands( *m_timer, 0u ) }
	{
	}

	LightPropagationPass::LightPropagationPass( Engine & engine
		, castor::String const & name
		, uint32_t gridSize
		, LightVolumePassResult const & injection
		, LightVolumePassResult const & accumulation
		, LightVolumePassResult const & propagate
		, LpvConfigUbo const & lpvConfigUbo )
		: LightPropagationPass{ engine
			, name
			, gridSize
			, nullptr
			, injection
			, accumulation
			, propagate
			, lpvConfigUbo }
	{
	}

	LightPropagationPass::LightPropagationPass( Engine & engine
		, castor::String const & name
		, uint32_t gridSize
		, TextureUnit const & geometry
		, LightVolumePassResult const & injection
		, LightVolumePassResult const & accumulation
		, LightVolumePassResult const & propagate
		, LpvConfigUbo const & lpvConfigUbo )
		: LightPropagationPass{ engine
			, name
			, gridSize
			, &geometry
			, injection
			, accumulation
			, propagate
			, lpvConfigUbo }
	{
	}

	ashes::Semaphore const & LightPropagationPass::compute( ashes::Semaphore const & toWait )const
	{
		auto & device = getCurrentRenderDevice( m_engine );
		RenderPassTimerBlock timerBlock{ m_timer->start() };
		timerBlock->notifyPassRender();
		auto * result = &toWait;

		device.graphicsQueue->submit( *m_commands.commandBuffer
			, toWait
			, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			, *m_commands.semaphore
			, nullptr );
		result = m_commands.semaphore.get();

		return *result;
	}

	CommandsSemaphore LightPropagationPass::getCommands( RenderPassTimer const & timer
		, uint32_t index )const
	{
		auto & device = getCurrentRenderDevice( m_engine );
		castor3d::CommandsSemaphore commands
		{
			device.graphicsCommandPool->createCommandBuffer( getName() ),
			device->createSemaphore( getName() )
		};
		auto & cmd = *commands.commandBuffer;

		cmd.begin( VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT );
		timer.beginPass( cmd, index );
		cmd.beginDebugBlock(
			{
				"Lighting - " + getName(),
				castor3d::makeFloatArray( m_engine.getNextRainbowColour() ),
			} );
		cmd.beginRenderPass( *m_renderPass
			, *m_frameBuffer
			, { getClearValue( LpvTexture::eR )
				, getClearValue( LpvTexture::eG )
				, getClearValue( LpvTexture::eB )
				, getClearValue( LpvTexture::eR )
				, getClearValue( LpvTexture::eG )
				, getClearValue( LpvTexture::eB ) }
			, VK_SUBPASS_CONTENTS_INLINE );
		cmd.bindPipeline( *m_pipeline );
		cmd.bindVertexBuffer( 0u, m_vertexBuffer->getBuffer(), 0u );
		cmd.bindDescriptorSet( *m_descriptorSet, *m_pipelineLayout );
		cmd.draw( m_count );
		cmd.endRenderPass();
		cmd.endDebugBlock();
		timer.endPass( cmd, index );
		cmd.end();

		return commands;
	}

	void LightPropagationPass::accept( PipelineVisitorBase & visitor )
	{
		visitor.visit( m_vertexShader );
		visitor.visit( m_geometryShader );
		visitor.visit( m_pixelShader );
	}
}
