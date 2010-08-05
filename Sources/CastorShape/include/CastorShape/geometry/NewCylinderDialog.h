//******************************************************************************
#ifndef ___CS_NEWCYLINDERDIALOG___
#define ___CS_NEWCYLINDERDIALOG___
//******************************************************************************
#include "Module_Geometry.h"
#include "NewGeometryDialog.h"
//******************************************************************************
class CSNewCylinderDialog : public CSNewGeometryDialog
{
public:
	CSNewCylinderDialog( wxWindow * parent, wxWindowID p_id);
	~CSNewCylinderDialog();

	float GetCylinderRadius()const;
	float GetCylinderHeight()const;
	int GetFacesNumber()const;
	String	GetFacesNumberStr()const;
};
//******************************************************************************
#endif
//******************************************************************************
