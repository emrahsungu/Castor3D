/*
See LICENSE file in root folder
*/
#ifndef ___C3D_BillboardList_H___
#define ___C3D_BillboardList_H___

#include "Castor3D/Material/MaterialModule.hpp"
#include "Castor3D/Scene/SceneModule.hpp"
#include "Castor3D/Shader/ShaderModule.hpp"

#include "Castor3D/Buffer/GeometryBuffers.hpp"
#include "Castor3D/Scene/MovableObject.hpp"
#include "Castor3D/Scene/RenderedObject.hpp"

#include <ashespp/Buffer/VertexBuffer.hpp>
#include <ashespp/Pipeline/PipelineVertexInputStateCreateInfo.hpp>

namespace castor3d
{
	class BillboardBase
		: public RenderedObject
	{
	public:
		/**
		 *\~english
		 *\brief		Constructor.
		 *\param[in]	scene			The parent scene.
		 *\param[in]	node			The parent scene node.
		 *\param[in]	vertexLayout	The layout for the vertex buffer.
		 *\param[in]	vertexBuffer	The vertex buffer.
		 *\~french
		 *\brief		Constructeur.
		 *\param[in]	scene			La scène parente.
		 *\param[in]	node			Le noeud de scène parent.
		 *\param[in]	vertexLayout	Le layout du tampon de sommets.
		 *\param[in]	vertexBuffer	Le tampon de sommets.
		 */
		C3D_API BillboardBase( Scene & scene
			, SceneNode * node
			, ashes::PipelineVertexInputStateCreateInfoPtr vertexLayout
			, uint32_t vertexStride
			, ashes::VertexBufferBasePtr vertexBuffer = nullptr );
		/**
		 *\~english
		 *\brief		Destructor.
		 *\~french
		 *\brief		Destructeur.
		 */
		C3D_API ~BillboardBase();
		/**
		 *\~english
		 *\brief		Initialises GPU side elements.
		 *\param[in]	count	The elements count.
		 *\return		\p true if all is OK.
		 *\~french
		 *\brief		Initialise les éléments GPU.
		 *\param[in]	count	Le nombre d'éléments.
		 *\return		\p true si tout s'est bien passé.
		 */
		C3D_API bool initialise( uint32_t count );
		/**
		 *\~english
		 *\brief		Cleans GPU side elements up
		 *\~french
		 *\brief		Nettoie les elements GPU
		 */
		C3D_API void cleanup();
		/**
		 *\~english
		 *\brief		Sorts the points from farthest to nearest from the camera.
		 *\param[in]	cameraPosition	The camera position, relative to billboard.
		 *\~french
		 *\brief		Trie les points des plus éloignés aux plus proches de la caméra.
		 *\param[in]	cameraPosition	La position de la caméra, relative au billboard.
		 */
		C3D_API void sortByDistance( castor::Point3f const & cameraPosition );
		/**
		 *\~english
		 *\brief		Updates the vertex buffer.
		 *\~french
		 *\brief		Met à jour le tampon de sommets.
		 */
		C3D_API void update();
		/**
		*\~english
		*name
		*	Getters.
		*\~french
		*name
		*	Accesseurs.
		*/
		/**@{*/
		inline MaterialSPtr getMaterial()const
		{
			return m_material.lock();
		}

		inline castor::Point2f const & getDimensions()const
		{
			return m_dimensions;
		}

		inline uint32_t getCount()const
		{
			return m_count;
		}

		inline bool isInitialised()const
		{
			return m_initialised;
		}

		inline ashes::VertexBufferBase const & getVertexBuffer()const
		{
			return *m_vertexBuffer;
		}

		inline ashes::VertexBufferBase & getVertexBuffer()
		{
			return *m_vertexBuffer;
		}

		inline GeometryBuffers const & getGeometryBuffers()const
		{
			return m_geometryBuffers;
		}

		inline Scene const & getParentScene()const
		{
			return m_scene;
		}

		inline Scene & getParentScene()
		{
			return m_scene;
		}

		inline SceneNode * getNode()const
		{
			return m_node;
		}

		inline BillboardType getBillboardType()const
		{
			return m_billboardType;
		}

		inline BillboardSize getBillboardSize()const
		{
			return m_billboardSize;
		}

		C3D_API ProgramFlags getProgramFlags()const;
		/**@}*/
		/**
		*\~english
		*name
		*	Setters.
		*\~french
		*name
		*	Mutateurs.
		*/
		/**@{*/
		inline void setDimensions( castor::Point2f const & value )
		{
			m_dimensions = value;
		}

		inline void setCenterOffset( uint32_t value )
		{
			m_centerOffset = value;
		}

		inline void setNode( SceneNode & value )
		{
			m_node = &value;
		}

		inline void setBillboardType( BillboardType value )
		{
			m_billboardType = value;
		}

		inline void setBillboardSize( BillboardSize value )
		{
			m_billboardSize = value;
		}

		C3D_API void setMaterial( MaterialSPtr value );
		C3D_API void setCount( uint32_t value );
		/**@}*/

		OnBillboardMaterialChanged onMaterialChanged;

	private:
		void doGatherBuffers( ashes::BufferCRefArray & buffers
			, std::vector< uint64_t > & offsets
			, ashes::PipelineVertexInputStateCreateInfoCRefArray & layouts );

	public:
		struct Vertex
		{
			castor::Point3f position;
			castor::Point2f uv;
		};
		using Quad = std::array< Vertex, 4u >;

	protected:
		Scene & m_scene;
		SceneNode * m_node;
		MaterialWPtr m_material;
		castor::Point2f m_dimensions;
		castor::Point3f m_cameraPosition;
		ashes::VertexBufferBasePtr m_vertexBuffer;
		ashes::PipelineVertexInputStateCreateInfoPtr m_vertexLayout;
		uint32_t m_vertexStride;
		ashes::VertexBufferPtr< Quad > m_quadBuffer;
		ashes::PipelineVertexInputStateCreateInfoPtr m_quadLayout;
		GeometryBuffers m_geometryBuffers;
		bool m_needUpdate{ true };
		bool m_initialised{ false };
		uint32_t m_count{ 0u };
		uint32_t m_centerOffset{ 0u };
		BillboardType m_billboardType{ BillboardType::eCylindrical };
		BillboardSize m_billboardSize{ BillboardSize::eDynamic };
	};

	class BillboardList
		: public MovableObject
		, public BillboardBase
	{
	public:
		/**
		\author		Sylvain DOREMUS
		\version	0.6.1.0
		\date		19/10/2011
		\~english
		\brief		Camera loader
		\~french
		\brief		Loader de Camera
		*/
		class TextWriter
			: public MovableObject::TextWriter
		{
		public:
			/**
			 *\~english
			 *\brief		Constructor
			 *\~french
			 *\brief		Constructeur
			 */
			C3D_API explicit TextWriter( castor::String const & tabs );
			/**
			 *\~english
			 *\brief		Writes a billboards list into a text file
			 *\param[in]	file	The file to save the object in
			 *\param[in]	obj	The object to save
			 *\~french
			 *\brief		Ecrit une camera dans un fichier texte
			 *\param[in]	file	Le fichier
			 *\param[in]	obj	L'objet
			 */
			C3D_API bool operator()( BillboardList const & obj
				, castor::TextFile & file );
		};

	public:
		/**
		 *\~english
		 *\brief		Constructor
		 *\param[in]	name		The name.
		 *\param[in]	scene		The parent scene.
		 *\param[in]	parent	The parent scene node.
		 *\~french
		 *\brief		Constructeur
		 *\param[in]	name		Le nom.
		 *\param[in]	scene		La scene parente.
		 *\param[in]	parent	Le noeud de scène parent.
		 */
		C3D_API BillboardList( castor::String const & name
			, Scene & scene
			, SceneNode & parent );
		/**
		 *\~english
		 *\brief		Constructor
		 *\param[in]	name		The name.
		 *\param[in]	scene		The parent scene.
		 *\param[in]	parent	The parent scene node.
		 *\~french
		 *\brief		Constructeur
		 *\param[in]	name		Le nom.
		 *\param[in]	scene		La scene parente.
		 *\param[in]	parent	Le noeud de scène parent.
		 */
		C3D_API BillboardList( castor::String const & name
			, Scene & scene );
		/**
		 *\~english
		 *\brief		Destructor
		 *\~french
		 *\brief		Destructeur
		 */
		C3D_API ~BillboardList();
		/**
		 *\~english
		 *\brief		Initialises GPU side elements
		 *\return		\p true if all is OK
		 *\~french
		 *\brief		Initialise les elements GPU
		 *\return		\p true si tout s'est bien passe
		 */
		C3D_API bool initialise();
		/**
		 *\~english
		 *\brief		Removes a point from the list
		 *\param[in]	index	The point index
		 *\~french
		 *\brief		Retire un point de la liste
		 *\param[in]	index	L'index du point
		 */
		C3D_API void removePoint( uint32_t index );
		/**
		 *\~english
		 *\brief		adds a point to the list
		 *\param[in]	position	The point
		 *\~french
		 *\brief		Ajoute un point a la liste
		 *\param[in]	position	Le point
		 */
		C3D_API void addPoint( castor::Point3f const & position );
		/**
		 *\~english
		 *\brief		adds a points list to the list
		 *\param[in]	positions	The points list
		 *\~french
		 *\brief		Ajoute une liste de points a la liste
		 *\param[in]	positions	La liste de points
		 */
		C3D_API void addPoints( castor::Point3fArray const & positions );
		/**
		 *\~english
		 *\brief		Attaches the movable object to a node
		 *\~french
		 *\brief		Attache l'object à un noeud
		 */
		C3D_API void attachTo( SceneNode & node )override;
		/**
		*\~english
		*name
		*	Getters.
		*\~french
		*name
		*	Accesseurs.
		*/
		/**@{*/
		inline castor::Point3f const & getAt( uint32_t index )const
		{
			return m_arrayPositions[index];
		}

		inline castor::Point3fArrayIt begin()
		{
			return m_arrayPositions.begin();
		}

		inline castor::Point3fArrayConstIt begin()const
		{
			return m_arrayPositions.begin();
		}

		inline castor::Point3fArrayIt end()
		{
			return m_arrayPositions.end();
		}

		inline castor::Point3fArrayConstIt end()const
		{
			return m_arrayPositions.end();
		}
		/**@}*/
		/**
		*\~english
		*name
		*	Setters.
		*\~french
		*name
		*	Mutateurs.
		*/
		/**@{*/
		inline void setAt( uint32_t index, castor::Point3f const & position )
		{
			m_needUpdate = true;
			m_arrayPositions[index] = position;
		}
		/**@}*/

	protected:
		castor::Point3fArray m_arrayPositions;
	};
}

#endif
