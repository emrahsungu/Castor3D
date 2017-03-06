/*
This source file is part of Castor3D (http://castor3d.developpez.com/castor3d.html)
Copyright (c) 2016 dragonjoker59@hotmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef __COMC3D_COM_VECTOR_3D_H__
#define __COMC3D_COM_VECTOR_3D_H__

#include "ComCastor3DPrerequisites.hpp"

#include "ComAtlObject.hpp"

#include <Math/Point.hpp>

namespace CastorCom
{
	/*!
	\author 	Sylvain DOREMUS
	\version	0.7.0
	\date		10/09/2014
	\~english
	\brief		This class defines a CVector3D object accessible from COM.
	\~french
	\brief		Cette classe définit un CVector3D accessible depuis COM.
	*/
	class ATL_NO_VTABLE CVector3D
		:	COM_ATL_OBJECT( Vector3D )
		,	public Castor::Point3r
	{
	public:
		/**
		 *\~english
		 *\brief		Default constructor.
		 *\~french
		 *\brief		Constructeur par défaut.
		 */
		CVector3D();
		/**
		 *\~english
		 *\brief		Destructor.
		 *\~french
		 *\brief		Destructeur.
		 */
		virtual ~CVector3D();

		COM_PROPERTY( X, FLOAT, make_getter( this, &Castor::Point3r::operator[], 0 ), make_putter( this, &Castor::Point3r::operator[], 0 ) );
		COM_PROPERTY( Y, FLOAT, make_getter( this, &Castor::Point3r::operator[], 1 ), make_putter( this, &Castor::Point3r::operator[], 1 ) );
		COM_PROPERTY( Z, FLOAT, make_getter( this, &Castor::Point3r::operator[], 2 ), make_putter( this, &Castor::Point3r::operator[], 2 ) );

		STDMETHOD( Negate )();
		STDMETHOD( Normalise )();
		STDMETHOD( Dot )( IVector3D * pVal, FLOAT * pRet );
		STDMETHOD( Cross )( /* [in] */ IVector3D * val, /* [out,retval] */ IVector3D ** pRet );
		STDMETHOD( Length )( /* [out,retval] */ FLOAT * pVal );
	};
	//!\~english Enters the ATL object into the object map, updates the registry and creates an instance of the object	\~french Ecrit l'objet ATL dans la table d'objets, met à jour le registre et crée une instance de l'objet
	OBJECT_ENTRY_AUTO( __uuidof( Vector3D ), CVector3D );

	DECLARE_VARIABLE_VAL_GETTER( Vector3D, Castor, Point3r );
	DECLARE_VARIABLE_REF_GETTER( Vector3D, Castor, Point3r );
	DECLARE_VARIABLE_REF_PUTTER( Vector3D, Castor, Point3r );
}

#endif