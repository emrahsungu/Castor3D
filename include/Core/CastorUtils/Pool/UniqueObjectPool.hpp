/*
See LICENSE file in root folder
*/
#ifndef ___CU_UNIQUE_OBJECT_POOL_H___
#define ___CU_UNIQUE_OBJECT_POOL_H___

#include "CastorUtils/Pool/ObjectPool.hpp"

namespace castor
{
	/*!
	\author		Sylvain DOREMUS
	\version	0.8.0
	\date		08/01/2016
	\~english
	\brief		Unique pool instance.
	\param		Object		The pool objects type.
	\param		MemDataType	The allocation/deallocation policy type.
	\~french
	\brief		Instance unique de pool.
	\param		Object		Le type des objets du pool.
	\param		MemDataType	Le type de la politique d'allocation/désallocation.
	*/
	template< typename Object, MemoryDataType MemDataType >
	class UniqueObjectPool
	{
		using MyObjectPool = ObjectPool< Object, MemDataType >;

	public:
		/**
		 *\~english
		 *\return		The unique object pool instance.
		 *\~french
		 *\return		L'instance unique du pool d'objets.
		 */
		static inline MyObjectPool & get()
		{
			return *doGetInstance();
		}
		/**
		 *\~english
		 *\brief		Creates the unique object pool instance.
		 *\param[in]	count	The object pool initial objects count.
		 *\~french
		 *\brief		Crée l'instance unique du pool d'objets.
		 *\param[in]	count	Le compte initial des objets du pool.
		 */
		static inline void create( uint32_t count = 256u )
		{
			if ( !doGetInstance() )
			{
				doGetInstance() = new MyObjectPool( count );
			}
		}
		/**
		 *\~english
		 *\brief		Destroys the unique object pool instance.
		 *\~french
		 *\brief		Détruit l'instance unique du pool d'objets.
		 */
		static inline void destroy()
		{
			if ( doGetInstance() )
			{
				delete doGetInstance();
				doGetInstance() = nullptr;
			}
		}

	private:
		static inline MyObjectPool *& doGetInstance()
		{
			static MyObjectPool * s_instance = nullptr;
			return s_instance;
		}
	};
	/*!
	\author		Sylvain DOREMUS
	\version	0.8.0
	\date		08/01/2016
	\~english
	\brief		Unique pool instance.
	\param		Object		The pool objects type.
	\param		MemDataType	The allocation/deallocation policy type.
	\~french
	\brief		Instance unique de pool.
	\param		Object		Le type des objets du pool.
	\param		MemDataType	Le type de la politique d'allocation/désallocation.
	*/
	template< typename Object, MemoryDataType MemDataType, uint32_t Align >
	class UniqueAlignedObjectPool
	{
		using MyAlignedObjectPool = AlignedObjectPool< Object, MemDataType, Align >;

	public:
		/**
		 *\~english
		 *\return		The unique object pool instance.
		 *\~french
		 *\return		L'instance unique du pool d'objets.
		 */
		static inline MyAlignedObjectPool & get()
		{
			return *doGetInstance();
		}
		/**
		 *\~english
		 *\brief		Creates the unique object pool instance.
		 *\param[in]	count	The object pool initial objects count.
		 *\~french
		 *\brief		Crée l'instance unique du pool d'objets.
		 *\param[in]	count	Le compte initial des objets du pool.
		 */
		static inline void create( uint32_t count = 256u )
		{
			if ( !doGetInstance() )
			{
				doGetInstance() = new MyAlignedObjectPool( count );
			}
		}
		/**
		 *\~english
		 *\brief		Destroys the unique object pool instance.
		 *\~french
		 *\brief		Détruit l'instance unique du pool d'objets.
		 */
		static inline void destroy()
		{
			if ( doGetInstance() )
			{
				delete doGetInstance();
				doGetInstance() = nullptr;
			}
		}

	private:
		static inline MyAlignedObjectPool *& doGetInstance()
		{
			static MyAlignedObjectPool * s_instance = nullptr;
			return s_instance;
		}
	};
}

#endif