#include "GuiCommon/Properties/TreeItems/RenderTargetTreeItemProperty.hpp"

#include "GuiCommon/Properties/AdditionalProperties.hpp"
#include "GuiCommon/Properties/TreeItems/PostEffectTreeItemProperty.hpp"
#include "GuiCommon/Properties/TreeItems/ToneMappingTreeItemProperty.hpp"
#include "GuiCommon/Shader/ShaderDialog.hpp"

#include <Castor3D/Render/RenderTarget.hpp>
#include <Castor3D/Render/PostEffect/PostEffect.hpp>
#include <Castor3D/Render/Technique/RenderTechnique.hpp>
#include <Castor3D/Render/ToneMapping/ToneMapping.hpp>

#include <wx/propgrid/advprops.h>

using namespace castor3d;
using namespace castor;

namespace GuiCommon
{
	namespace
	{
		static wxString PROPERTY_CATEGORY_RENDER_TARGET = _( "Render Target: " );
		static wxString PROPERTY_RENDER_TARGET_SHADER = _( "Shader" );
		static wxString PROPERTY_RENDER_TARGET_EDIT_SHADER = _( "View Shaders..." );
		static wxString PROPERTY_RENDER_TARGET_SSAO = _( "SSAO" );
		static wxString PROPERTY_RENDER_TARGET_SSAO_ENABLED = _( "Enable SSAO" );
		static wxString PROPERTY_RENDER_TARGET_SSAO_HIGH_QUALITY = _( "High Quality" );
		static wxString PROPERTY_RENDER_TARGET_SSAO_NORMALS_BUFFER = _( "Normals Buffer" );
		static wxString PROPERTY_RENDER_TARGET_SSAO_RADIUS = _( "Radius" );
		static wxString PROPERTY_RENDER_TARGET_SSAO_BIAS = _( "Bias" );
		static wxString PROPERTY_RENDER_TARGET_SSAO_INTENSITY = _( "Intensity" );
		static wxString PROPERTY_RENDER_TARGET_SSAO_SAMPLES = _( "Samples" );
		static wxString PROPERTY_RENDER_TARGET_SSAO_EDGE_SHARPNESS = _( "Edge Sharpness" );
		static wxString PROPERTY_RENDER_TARGET_SSAO_BLUR_HIGH_QUALITY = _( "High Quality Blur" );
		static wxString PROPERTY_RENDER_TARGET_SSAO_BLUR_STEP_SIZE = _( "Blur Step Size" );
		static wxString PROPERTY_RENDER_TARGET_SSAO_BLUR_RADIUS = _( "Blur Radius" );
		static wxString PROPERTY_RENDER_TARGET_DEBUG_VIEW = _( "Debug View" );
	}

	void AppendRenderTarget( wxTreeCtrlBase * list
		, bool editable
		, wxTreeItemId id
		, RenderTarget & target )
	{
		auto targetId = list->AppendItem( id
			, make_wxString( target.getName() )
			, eBMP_RENDER_TARGET
			, eBMP_RENDER_TARGET_SEL
			, new RenderTargetTreeItemProperty( editable, target ) );

		for ( auto postEffect : target.getHDRPostEffects() )
		{
			wxTreeItemId idPostEffect = list->AppendItem( targetId
				, _( "HDR - " ) + make_wxString( postEffect->getFullName() )
				, eBMP_POST_EFFECT
				, eBMP_POST_EFFECT_SEL
				, new PostEffectTreeItemProperty( editable, *postEffect, list ) );
		}

		for ( auto postEffect : target.getSRGBPostEffects() )
		{
			wxTreeItemId idPostEffect = list->AppendItem( targetId
				, _( "SRGB - " ) + make_wxString( postEffect->getFullName() )
				, eBMP_POST_EFFECT
				, eBMP_POST_EFFECT_SEL
				, new PostEffectTreeItemProperty( editable, *postEffect, list ) );
		}

		auto toneMapping = target.getToneMapping();

		if ( toneMapping )
		{
			wxTreeItemId idToneMapping = list->AppendItem( targetId
				, make_wxString( toneMapping->getFullName() )
				, eBMP_TONE_MAPPING
				, eBMP_TONE_MAPPING_SEL
				, new ToneMappingTreeItemProperty( editable, *toneMapping, list ) );
		}
	}

	RenderTargetTreeItemProperty::RenderTargetTreeItemProperty( bool editable
		, RenderTarget & target )
		: TreeItemProperty( target.getEngine(), editable, ePROPERTY_DATA_TYPE_RENDER_TARGET )
		, m_target( target )
	{
		PROPERTY_CATEGORY_RENDER_TARGET = _( "Render Target: " );
		PROPERTY_RENDER_TARGET_SHADER = _( "Shader" );
		PROPERTY_RENDER_TARGET_EDIT_SHADER = _( "View Shaders..." );
		PROPERTY_RENDER_TARGET_SSAO = _( "SSAO" );
		PROPERTY_RENDER_TARGET_SSAO_ENABLED = _( "Enable SSAO" );
		PROPERTY_RENDER_TARGET_SSAO_HIGH_QUALITY = _( "High Quality" );
		PROPERTY_RENDER_TARGET_SSAO_NORMALS_BUFFER = _( "Normals Buffer" );
		PROPERTY_RENDER_TARGET_SSAO_RADIUS = _( "Radius" );
		PROPERTY_RENDER_TARGET_SSAO_BIAS = _( "Bias" );
		PROPERTY_RENDER_TARGET_SSAO_INTENSITY = _( "Intensity" );
		PROPERTY_RENDER_TARGET_SSAO_SAMPLES = _( "Samples" );
		PROPERTY_RENDER_TARGET_SSAO_EDGE_SHARPNESS = _( "Edge Sharpness" );
		PROPERTY_RENDER_TARGET_SSAO_BLUR_HIGH_QUALITY = _( "High Quality Blur" );
		PROPERTY_RENDER_TARGET_SSAO_BLUR_STEP_SIZE = _( "Blur Step Size" );
		PROPERTY_RENDER_TARGET_SSAO_BLUR_RADIUS = _( "Blur Radius" );
		PROPERTY_RENDER_TARGET_DEBUG_VIEW = _( "Debug View" );

		CreateTreeItemMenu();
	}

	RenderTargetTreeItemProperty::~RenderTargetTreeItemProperty()
	{
	}

	void RenderTargetTreeItemProperty::doCreateProperties( wxPGEditor * editor
		, wxPropertyGrid * grid )
	{
		auto & target = getRenderTarget();
		wxString TARGETS[] =
		{
			_( "Window" ),
			_( "Texture" )
		};

		grid->Append( new wxPropertyCategory( PROPERTY_CATEGORY_RENDER_TARGET + TARGETS[size_t( target.getTargetType() )] ) );

		for ( auto & postEffect : target.getHDRPostEffects() )
		{
			grid->Append( new wxPropertyCategory( PROPERTY_CATEGORY_RENDER_TARGET + TARGETS[size_t( target.getTargetType() )] ) );
		}

		auto & debugConfig = target.getTechnique()->getDebugConfig();
		wxPGChoices choices;

		for ( auto & intermediate : target.getIntermediateViews() )
		{
			choices.Add( make_wxString( intermediate.name ) );
		}

		auto selected = choices.GetLabels()[size_t( debugConfig.debugIndex )];
		grid->Append( new wxEnumProperty( PROPERTY_RENDER_TARGET_DEBUG_VIEW, PROPERTY_RENDER_TARGET_DEBUG_VIEW, choices ) )->SetValue( selected );

		auto & ssaoConfig = target.getTechnique()->getSsaoConfig();
		grid->Append( new wxPropertyCategory( PROPERTY_RENDER_TARGET_SSAO ) );
		grid->Append( new wxBoolProperty( PROPERTY_RENDER_TARGET_SSAO_ENABLED, PROPERTY_RENDER_TARGET_SSAO_ENABLED, ssaoConfig.enabled ) );
		grid->Append( new wxBoolProperty( PROPERTY_RENDER_TARGET_SSAO_HIGH_QUALITY, PROPERTY_RENDER_TARGET_SSAO_HIGH_QUALITY, ssaoConfig.highQuality ) );
		grid->Append( new wxBoolProperty( PROPERTY_RENDER_TARGET_SSAO_NORMALS_BUFFER, PROPERTY_RENDER_TARGET_SSAO_NORMALS_BUFFER, ssaoConfig.useNormalsBuffer ) );
		grid->Append( new wxFloatProperty( PROPERTY_RENDER_TARGET_SSAO_RADIUS, PROPERTY_RENDER_TARGET_SSAO_RADIUS, ssaoConfig.radius ) );
		grid->Append( new wxFloatProperty( PROPERTY_RENDER_TARGET_SSAO_BIAS, PROPERTY_RENDER_TARGET_SSAO_BIAS, ssaoConfig.bias ) );
		grid->Append( new wxFloatProperty( PROPERTY_RENDER_TARGET_SSAO_INTENSITY, PROPERTY_RENDER_TARGET_SSAO_INTENSITY, ssaoConfig.intensity ) );
		grid->Append( new wxUIntProperty( PROPERTY_RENDER_TARGET_SSAO_SAMPLES, PROPERTY_RENDER_TARGET_SSAO_SAMPLES, ssaoConfig.numSamples ) );
		grid->Append( new wxFloatProperty( PROPERTY_RENDER_TARGET_SSAO_EDGE_SHARPNESS, PROPERTY_RENDER_TARGET_SSAO_EDGE_SHARPNESS, ssaoConfig.edgeSharpness ) );
		grid->Append( new wxBoolProperty( PROPERTY_RENDER_TARGET_SSAO_BLUR_HIGH_QUALITY, PROPERTY_RENDER_TARGET_SSAO_BLUR_HIGH_QUALITY, ssaoConfig.blurHighQuality ) );
		grid->Append( new wxUIntProperty( PROPERTY_RENDER_TARGET_SSAO_BLUR_STEP_SIZE, PROPERTY_RENDER_TARGET_SSAO_BLUR_STEP_SIZE, ssaoConfig.blurStepSize ) );
		grid->Append( new wxIntProperty( PROPERTY_RENDER_TARGET_SSAO_BLUR_RADIUS, PROPERTY_RENDER_TARGET_SSAO_BLUR_RADIUS, ssaoConfig.blurRadius->value() ) );
	}

	void RenderTargetTreeItemProperty::doPropertyChange( wxPropertyGridEvent & event )
	{
		auto & target = getRenderTarget();
		wxPGProperty * property = event.GetProperty();

		if ( property )
		{
			auto & ssaoConfig = target.getTechnique()->getSsaoConfig();
			auto & debugConfig = target.getTechnique()->getDebugConfig();

			if ( property->GetName() == PROPERTY_RENDER_TARGET_SSAO_ENABLED )
			{
				onValueChange( bool( property->GetValue() ), &ssaoConfig.enabled );
			}
			else if ( property->GetName() == PROPERTY_RENDER_TARGET_SSAO_HIGH_QUALITY )
			{
				onValueChange( bool( property->GetValue() ), &ssaoConfig.highQuality );
			}
			else if ( property->GetName() == PROPERTY_RENDER_TARGET_SSAO_NORMALS_BUFFER )
			{
				onValueChange( bool( property->GetValue() ), &ssaoConfig.useNormalsBuffer );
			}
			else if ( property->GetName() == PROPERTY_RENDER_TARGET_SSAO_RADIUS )
			{
				onValueChange( float( property->GetValue().GetDouble() ), &ssaoConfig.radius );
			}
			else if ( property->GetName() == PROPERTY_RENDER_TARGET_SSAO_BIAS )
			{
				onValueChange( float( property->GetValue().GetDouble() ), &ssaoConfig.bias );
			}
			else if ( property->GetName() == PROPERTY_RENDER_TARGET_SSAO_INTENSITY )
			{
				onValueChange( float( property->GetValue().GetDouble() ), &ssaoConfig.intensity );
			}
			else if ( property->GetName() == PROPERTY_RENDER_TARGET_SSAO_SAMPLES )
			{
				onValueChange( uint32_t( property->GetValue().GetLong() ), &ssaoConfig.numSamples );
			}
			else if ( property->GetName() == PROPERTY_RENDER_TARGET_SSAO_EDGE_SHARPNESS )
			{
				onValueChange( float( property->GetValue().GetDouble() ), &ssaoConfig.edgeSharpness );
			}
			else if ( property->GetName() == PROPERTY_RENDER_TARGET_SSAO_BLUR_HIGH_QUALITY )
			{
				onValueChange( bool( property->GetValue() ), &ssaoConfig.blurHighQuality );
			}
			else if ( property->GetName() == PROPERTY_RENDER_TARGET_SSAO_BLUR_STEP_SIZE )
			{
				onValueChange( uint32_t( property->GetValue().GetLong() ), &ssaoConfig.blurStepSize );
			}
			else if ( property->GetName() == PROPERTY_RENDER_TARGET_SSAO_BLUR_RADIUS )
			{
				onValueChange( int32_t( property->GetValue().GetLong() ), &ssaoConfig.blurRadius );
			}
			else if ( property->GetName() == PROPERTY_RENDER_TARGET_DEBUG_VIEW )
			{
				onValueChange( uint32_t( property->GetChoiceSelection() ), &debugConfig.debugIndex );
			}
		}
	}
}
