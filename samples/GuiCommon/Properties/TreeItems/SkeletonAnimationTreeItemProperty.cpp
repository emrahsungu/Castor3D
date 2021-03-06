#include "GuiCommon/Properties/TreeItems/SkeletonAnimationTreeItemProperty.hpp"

#include "GuiCommon/Properties/AdditionalProperties.hpp"

#include <Castor3D/Animation/Animable.hpp>
#include <Castor3D/Model/Skeleton/Animation/SkeletonAnimation.hpp>
#include <Castor3D/Scene/Scene.hpp>

#include <wx/propgrid/advprops.h>

using namespace castor3d;
using namespace castor;

namespace GuiCommon
{
	namespace
	{
		static wxString PROPERTY_CATEGORY_SKELETON_ANIMATION = _( "Skeleton Animation: " );
	}

	SkeletonAnimationTreeItemProperty::SkeletonAnimationTreeItemProperty( bool editable, SkeletonAnimation & animation )
		: TreeItemProperty( animation.getOwner()->getScene()->getEngine(), editable, ePROPERTY_DATA_TYPE_SKELETON_ANIMATION )
		, m_animation( animation )
	{
		PROPERTY_CATEGORY_SKELETON_ANIMATION = _( "Skeleton Animation: " );

		CreateTreeItemMenu();
	}

	SkeletonAnimationTreeItemProperty::~SkeletonAnimationTreeItemProperty()
	{
	}

	void SkeletonAnimationTreeItemProperty::doCreateProperties( wxPGEditor * editor, wxPropertyGrid * grid )
	{
		grid->Append( new wxPropertyCategory( PROPERTY_CATEGORY_SKELETON_ANIMATION + wxString( m_animation.getName() ) ) );
	}

	void SkeletonAnimationTreeItemProperty::doPropertyChange( wxPropertyGridEvent & p_event )
	{
		wxPGProperty * property = p_event.GetProperty();

		if ( property )
		{
		}
	}
}
