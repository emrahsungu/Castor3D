/*
See LICENSE file in root folder
*/
#ifndef ___C3D_FrameListener_H___
#define ___C3D_FrameListener_H___

#include "FrameEventModule.hpp"

#include <CastorUtils/Design/Named.hpp>

namespace castor3d
{
	class FrameListener
		: public castor::Named
	{
	public:
		/**
		 *\~english
		 *\brief		Constructor.
		 *\param[in]	name	The listener's name.
		 *\~french
		 *\brief		Constructeur.
		 *\param[in]	name	Le nom du listener.
		 */
		C3D_API explicit FrameListener( castor::String const & name );
		/**
		 *\~english
		 *\brief		Destructor.
		 *\~french
		 *\brief		Destructeur.
		 */
		C3D_API ~FrameListener();
		/**
		 *\~english
		 *\brief		Empties the event lists.
		 *\~french
		 *\brief		Vide les listes d'évènements.
		 */
		C3D_API void flush();
		/**
		 *\~english
		 *\brief		Puts an event in the corresponding array.
		 *\param[in]	event	The event to put.
		 *\~french
		 *\brief		Ajoute un évènement à la liste d'évènements correspondant à sont type.
		 *\param[in]	event	L'évènement à ajouter.
		 */
		C3D_API void postEvent( FrameEventUPtr && event );
		/**
		 *\~english
		 *\brief		Applies all events of a given type, then discards them.
		 *\param[in]	type	The type of events to fire.
		 *\return		\p true si tous les évènements se sont exécutés sans erreur.
		 *\~french
		 *\brief		Traite tous les évènements d'un type donné.
		 *\param[in]	type	Le type des évènements à traiter.
		 *\return		\p true if all events were processed successfully.
		 */
		C3D_API bool fireEvents( EventType type );
		/**
		 *\~english
		 *\brief		Discards all events of a given type.
		 *\param[in]	type	The type of events to fire.
		 *\~french
		 *\brief		Supprime tous les évènements d'un type donné.
		 *\param[in]	type	Le type des évènements à traiter.
		 */
		C3D_API void flushEvents( EventType type );

	protected:
		/**
		 *\~english
		 *\brief		Empties the event lists.
		 *\~french
		 *\brief		Vide les listes d'évènements.
		 */
		C3D_API virtual void doFlush() {}

	protected:
		//!\~english	The events arrays.
		//!\~french		Les tableaux d'évènements.
		std::array< FrameEventPtrArray,	size_t( EventType::eCount ) > m_events;
		//!\~english	Mutex to make this class thread safe.
		//!\~french		Mutex pour rendre cette classe thread safe.
		std::recursive_mutex m_mutex;
	};
}

#endif
