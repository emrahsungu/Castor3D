/*
See LICENSE file in root folder
*/
#ifndef ___C3D_FunctorEvent_H___
#define ___C3D_FunctorEvent_H___

#include "FrameEvent.hpp"

namespace castor3d
{
	template< class Functor >
	class FunctorEvent
		: public FrameEvent
	{
	private:
		/**
		 *\~english
		 *\brief		Copy constructor
		 *\param[in]	copy	The object to copy
		 *\~french
		 *\brief		Constructeur par copie
		 *\param[in]	copy	L'objet à copier
		 */
		inline FunctorEvent( FunctorEvent const & copy ) = delete;
		/**
		 *\~english
		 *\brief		Copy assignment operator
		 *\param[in]	copy	The object to copy
		 *\~french
		 *\brief		Opérateur d'affectation par copie
		 *\param[in]	copy	L'objet à copier
		 */
		inline FunctorEvent & operator=( FunctorEvent const & copy ) = delete;

	public:
		/**
		 *\~english
		 *\brief		Constructor
		 *\param[in]	type	The event type
		 *\param[in]	functor	The functor to execute
		 *\~french
		 *\brief		Constructeur
		 *\param[in]	type	Le type d'évènement
		 *\param[in]	functor	Le foncteur à exécuter
		 */
		inline FunctorEvent( EventType type
			, Functor functor )
			: FrameEvent( type )
			, m_functor( functor )
		{
		}
		/**
		 *\~english
		 *\brief		Destructor
		 *\~french
		 *\brief		Destructeur
		 */
		inline ~FunctorEvent()
		{
		}
		/**
		 *\~english
		 *\brief		Applies the event
		 *\remarks		Executes the function
		 *\return		\p true if the event was applied successfully
		 *\~french
		 *\brief		Traite l'évènement
		 *\remarks		Exécute la fonction
		 *\return		\p true si l'évènement a été traité avec succès
		 */
		virtual bool apply()
		{
			m_functor();
			return true;
		}

	private:
		//!\~english The functor to execute	\~french Le foncteur à exécuter
		Functor m_functor;
	};
	/**
	 *\~english
	 *\brief		Helper function to create a functor event
	 *\param[in]	type		The event type
	 *\param[in]	functor	The functor to execute
	 *\~french
	 *\brief		Constructeur
	 *\param[in]	type		Le type d'évènement
	 *\param[in]	functor	Le foncteur à exécuter
	 */
	template< typename Functor >
	inline std::unique_ptr< FunctorEvent< Functor > > makeFunctorEvent( EventType type
		, Functor functor )
	{
		return std::make_unique< FunctorEvent< Functor > >( type, functor );
	}
}

#endif
