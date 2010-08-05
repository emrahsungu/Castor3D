/*
This source file is part of Castor3D (http://dragonjoker.co.cc

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*/
#ifndef ___C3D_Scene___
#define ___C3D_Scene___

#include "Module_Scene.h"
#include "../geometry/Module_Geometry.h"
#include "../light/Light.h"
#include "../geometry/mesh/Mesh.h"

namespace Castor3D
{
	//! Scene handler class
	/*!
	A scene is a collection of lights, scene nodes and geometries. It has at least one camera to render it
	\author Sylvain DOREMUS
	\version 0.1
	\date 09/02/2010
	*/
	class CS3D_API Scene
	{
	private:
		String m_name;							//!< The scene name
		SceneNode * m_rootNode;					//!< The root scene node, each node is attached at this if not attached to another one
		Camera * m_rootCamera;					//!< The root camera, necessary for any render

		LightStrMap m_addedLights;				//!< The lights map
		SceneNodeStrMap m_addedNodes;			//!< The nodes map
		GeometryStrMap m_addedPrimitives;		//!< The geometries map
		CameraStrMap m_addedCameras;			//!< The other cameras

		LightPtrArray m_arrayLightsToDelete;			//!< The lights to delete array
		SceneNodePtrArray m_arrayNodesToDelete;			//!< The nodes to delete array
		GeometryPtrArray m_arrayPrimitivesToDelete;		//!< The geometries to delete array
		CameraPtrArray m_arrayCamerasToDelete;			//!< The cameras to delete array

		GeometryStrMap m_newlyAddedPrimitives;	//!< The newly added geometries, this map is used to make the vertex buffer of them, then they are removed from this list.

		size_t m_nbFaces;						//!< The number of faces in this scene
		size_t m_nbVertex;						//!< The number of vertexes in this scene

		bool m_changed;							//!< Tells if the scene has changed, id est if a geometry has been created or added to it => Vertex buffers need to be generated

		NormalsMode m_normalsMode;

		Geometry * m_selectedGeometry;

	public:
		/**
		 * Constructor
		 *@param p_name : [in] The scene name
		 */
		Scene( const String & p_name);
		/**
		 * Destructor
		 */
		~Scene();
		/**
		 * Clears the maps, leaves the root node and the root camera
		 */
		void ClearScene();
		/**
		 * Renders the scene in a given display mode
		 *@param p_displayMode : [in] The mode in which the display must be made
		 *@param p_tslf : [in] The time elapsed since the last frame was rendered
		 */
		void Render( DrawType p_displayMode, float p_tslf);
		/**
		 * Creates a scene node in this scene, attached to the root node if th given parent is NULL
		 *@param p_name : [in] The node name, default is empty
		 *@param p_parent : [in] The parent node, if NULL, the created node will be attached to root
		 */
		SceneNode * CreateSceneNode( const String & p_name, SceneNode * p_parent = NULL);
		/**
		 * Creates a primitive, given a MeshType and the primitive definitions
		 *@param p_name : [in] The primitive name
		 *@param p_type : [in] The primitive mesh type (plane, cube, torus...)
		 *@param p_meshName : [in] The mesh name, creates a new mesh if it doesn't exist
		 *@param p_faces : [in] The faces numbers
		 *@param p_size : [in] The geometry dimensions
		 */
		Geometry * CreatePrimitive( const String & p_name, Mesh::eTYPE p_type,
									const String & p_meshName, UIntArray p_faces,
									FloatArray p_size);
		/**
		 * Creates a camera
		 *@param p_name : [in] The camera name
		 *@param p_ww, p_wh : [in] The window size
		 *@param p_type : [in] The viewport projection type
		 */
		Camera * CreateCamera( const String & p_name, int p_ww, int p_wh,
							   ProjectionType p_type);
		/**
		* Creates a light
		*@param p_name : [in] The light name
		*@param p_index : [in] The light index
		*@param p_type : [in] The light type
		 */
		Light * CreateLight( Light::eTYPE p_type, const String & p_name);
		/**
		 * Creates the vertex buffers in a given normals mode, and tells if the face's or vertex's normals are shown
		 *@param p_nm : [in] The normals mode (face or vertex)
		 *@param p_showNormals : [in] Whether or not to show the face or vertex normals
		 */
		void CreateList( NormalsMode p_nm, bool p_showNormals);
		/**
		 * Retrieves the node with the given name
		 *@param p_name : [in] The name of the node
		 *@return The named node, NULL if not found
		 */
		SceneNode * GetNode( const String & p_name)const;
		SceneNodeStrMap::iterator GetNodesIterator() { return m_addedNodes.begin(); }
		SceneNodeStrMap::const_iterator GetNodesEnd() { return m_addedNodes.end(); }
		LightStrMap::iterator GetLightsIterator() { return m_addedLights.begin(); }
		LightStrMap::const_iterator GetLightsEnd() { return m_addedLights.end(); }
		GeometryStrMap::iterator GetGeometriesIterator() { return m_addedPrimitives.begin(); }
		GeometryStrMap::const_iterator GetGeometriesEnd() { return m_addedPrimitives.end(); }
		/**
		 * Adds a node to the scene
		 *@param p_node : [in] The node to add
		 */
		void AddNode( SceneNode * p_node);
		/**
		 * Adds a light to the scene
		 *@param p_light : [in] The light to add
		 */
		void AddLight( Light * p_light);
		/**
		 * Adds a geometry to the scene
		 *@param p_geometry : [in] The geometry to add
		 */
		void AddGeometry( Geometry * p_geometry);
		/**
		 * Retrieves the geometry with the given name
		 *@param p_name : [in] the name of the geometry
		 *@return The named geometry, NULL if not found
		 */
		Geometry * GetGeometry( String p_name);
		/**
		 * Removes the geometry given in argument from the scene and deletes it
		 *@param p_geometry : [in] the geometry to remove
		 */
		void RemoveLight( Light * p_pLight);
		/**
		 * Removes the geometry given in argument from the scene and deletes it
		 *@param p_geometry : [in] the geometry to remove
		 */
		void RemoveNode( SceneNode * p_pNode);
		/**
		 * Removes the geometry given in argument from the scene and deletes it
		 *@param p_geometry : [in] the geometry to remove
		 */
		void RemoveGeometry( Geometry * p_pGeometry);
		/**
		 * Removes the geometry given in argument from the scene and deletes it
		 *@param p_geometry : [in] the geometry to remove
		 */
		void RemoveCamera( Camera * p_pCamera);
		/**
		 * Removes the geometry given in argument from the scene and deletes it
		 *@param p_geometry : [in] the geometry to remove
		 */
		void RemoveAllLights();
		/**
		 * Removes the geometry given in argument from the scene and deletes it
		 *@param p_geometry : [in] the geometry to remove
		 */
		void RemoveAllNodes();
		/**
		 * Removes the geometry given in argument from the scene and deletes it
		 *@param p_geometry : [in] the geometry to remove
		 */
		void RemoveAllGeometries();
		/**
		 * Removes the geometry given in argument from the scene and deletes it
		 *@param p_geometry : [in] the geometry to remove
		 */
		void RemoveAllCameras();
		/**
		 * Retrieves a map of visibility of the geometries, sorted by geometry name
		 *@return The visibility map
		 */
		std::map <String, bool> GetGeometriesVisibility();
		/**
		 * Writes the scene in a file
		 *@param p_file : [in] file to write in
		 *@return true if successful, false if not
		 */
		bool Write( General::Utils::File & p_file)const;
		/**
		 * Reads the scene from a file
		 *@param p_file : [in] file to read from
		 *@return true if successful, false if not
		 */
		bool Read( General::Utils::File & p_file);
		/**
		* Reads the scene from a 3DS file
		*@param p_file : [in] file to read from
		*@return true if successful, false if not
		*/
		bool Import3DS( const String & p_file);
		/**
		* Reads the scene from a Obj file
		*@param p_file : [in] file to read from
		*@return true if successful, false if not
		*/
		bool ImportObj( const String & p_file);
		/**
		* Reads the scene from a MD2 (Quake 2 Model) file
		*@param p_file : [in] file to read from
		*@return true if successful, false if not
		*/
		bool ImportMD2( const String & p_file, const String & p_texName=C3DEmptyString);
		/**
		* Reads the scene from a MD3 (Quake 3 Model) file
		*@param p_file : [in] file to read from
		*@return true if successful, false if not
		*/
		bool ImportMD3( const String & p_file, const String & p_texName=C3DEmptyString);
		/**
		* Reads the scene from a ASE (ASCII Scene Export) file
		*@param p_file : [in] file to read from
		*@return true if successful, false if not
		*/
		bool ImportASE( const String & p_file);
		/**
		* Reads the scene from a PLY () file
		*@param p_file : [in] file to read from
		*@return true if successful, false if not
		*/
		bool ImportPLY( const String & p_file);
		/**
		* Reads the scene from a BSP (Quake 3 Map) file
		*@param p_file : [in] file to read from
		*@return true if successful, false if not
		*/
		bool ImportBSP( const String & p_file);

		void Select( Ray * p_ray, Geometry ** p_geo, Submesh ** p_submesh, Face ** p_face, Vector3f ** p_vertex);

	private:
		/**
		 * Writes the lights in a file
		 *@param p_file : [in] the file to write in
		 *@return true if successful, false if not
		 */
		bool _writeLights( General::Utils::File & p_file)const;
		/**
		 * Reads the lights from a file
		 *@param p_file : [in] the file to read from
		 *@return true if successful, false if not
		 */
		bool _readLights( General::Utils::File & p_file);
		/**
		 * Writes the geometries in a file
		 *@param p_file : [in] the file to write in
		 *@return true if successful, false if not
		 */
		bool _writeGeometries( General::Utils::File & p_file)const;
		/**
		 * Reads the geometries from a file
		 *@param p_file : [in] the file to read from
		 *@return true if successful, false if not
		 */
		bool _readGeometries( General::Utils::File & p_file);
		bool _importExternal( const String & p_fileName, ExternalImporter * p_importer);
		/**
		 * Deletes objects which need to be deleted
		 */
		void _deleteToDelete();

	public:
		/**
		 * @return The scene name
		 */
		inline String			GetName			()const { return m_name; }
		/**
		 * @return The root node
		 */
		inline SceneNode *		GetRootNode		()const { return m_rootNode; }
		/**
		 * @return The root camera
		 */
		inline Camera *			GetRootCamera	()const { return m_rootCamera; }
		/**
		 * @return The geometries number
		 */
		inline size_t			GetNbGeometries	()const { return m_addedPrimitives.size(); }
		/**
		 * @return Tells if the name has changed
		 */
		inline bool				HasChanged		()const { return m_changed; }
		/**
		 * @return The lights
		 */
		inline LightStrMap		GetLights		()const { return m_addedLights; }
	};
}

#endif

