#include "PointProperties.hpp"

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

using namespace Castor;

GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point2b, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point3b, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point4b, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point2i, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point3i, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point4i, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point2ui, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point3ui, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point4ui, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point2f, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point3f, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point4f, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point2d, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point3d, 16, wxEMPTY_PARAMETER_VALUE )
GC_PG_IMPLEMENT_ALIGNED_VARIANT_DATA_EXPORTED_DUMMY_EQ( Point4d, 16, wxEMPTY_PARAMETER_VALUE )

namespace GuiCommon
{
	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point2bProperty, wxPGProperty, Point2b, Point2b const &, TextCtrl )
	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point3bProperty, wxPGProperty, Point3b, Point3b const &, TextCtrl )
	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point4bProperty, wxPGProperty, Point4b, Point4b const &, TextCtrl )

	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point2iProperty, wxPGProperty, Point2i, Point2i const &, TextCtrl )
	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point3iProperty, wxPGProperty, Point3i, Point3i const &, TextCtrl )
	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point4iProperty, wxPGProperty, Point4i, Point4i const &, TextCtrl )

	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point2uiProperty, wxPGProperty, Point2ui, Point2ui const &, TextCtrl )
	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point3uiProperty, wxPGProperty, Point3ui, Point3ui const &, TextCtrl )
	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point4uiProperty, wxPGProperty, Point4ui, Point4ui const &, TextCtrl )

	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point2fProperty, wxPGProperty, Point2f, Point2f const &, TextCtrl )
	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point3fProperty, wxPGProperty, Point3f, Point3f const &, TextCtrl )
	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point4fProperty, wxPGProperty, Point4f, Point4f const &, TextCtrl )

	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point2dProperty, wxPGProperty, Point2d, Point2d const &, TextCtrl )
	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point3dProperty, wxPGProperty, Point3d, Point3d const &, TextCtrl )
	GC_PG_IMPLEMENT_PROPERTY_CLASS( Point4dProperty, wxPGProperty, Point4d, Point4d const &, TextCtrl )
}