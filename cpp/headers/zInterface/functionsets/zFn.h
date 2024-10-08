// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>
//

#ifndef ZSPACE_FN_H
#define ZSPACE_FN_H

#pragma once


#include <headers/zInterface/objects/zObj.h>

#include <depends/nlohmann/json.hpp>
using json = nlohmann::json;;

using namespace std;

#if defined ZSPACE_USD_INTEROP
#include <pxr/pxr.h>
#include <pxr/usd/usd/schemaBase.h>
#include <pxr/usd/usd/prim.h>

#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/subset.h>

#if PXR_VERSION >= 2208
#include <pxr/usd/usdGeom/primvarsAPI.h>
#endif

PXR_NAMESPACE_USING_DIRECTIVE;
#endif

namespace zSpace
{
	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zFuntionSets
	*	\brief The function set classes of the library.
	*  @{
	*/

	/*! \class zFn
	*	\brief The base function set class.
	*	\since version 0.0.2
	*/

	/** @}*/

	/** @}*/
	class ZSPACE_API zFn
	{

	protected:
		
		/*!	\brief function type  */
		zFnType fnType;

		/*!	\brief core utilities Object  */
		zUtilsCore coreUtils;
		

	public:
		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zFn();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zFn();

		//--------------------------
		//---- VIRTUAL METHODS
		//--------------------------

		/*! \brief This method return the function set type.
		*
		*	\return 	zFnType			- type of function set.
		*	\since version 0.0.2
		*/
		virtual zFnType getType() = 0;

		/*! \brief This method imports the object linked to function type.
		*
		*	\param	[in]	path			- input file name including the directory path and extension.
		*	\param	[in]	type			- type of file to be imported.
		*	\param	[in]	staticGeom		- true if the object is static. Helps speed up display especially for meshes object. Default set to false.
		*	\since version 0.0.2
		*/
		virtual void from(std::string path, zFileTpye type, bool staticGeom = false) = 0;

		/*! \brief This method imports the object linked to function type.
		*
		*	\param	[in]	j				- input json file.
		*	\param	[in]	staticGeom		- true if the object is static. Helps speed up display especially for meshes object. Default set to false.
		*	\since version 0.0.2
		*/
		virtual void from(json &j, bool staticGeom = false) = 0;		

		/*! \brief This method exports the object linked to output file.
		*
		*	\param [in]		path			- output file name including the directory path and extension.
		*	\param [in]		type			- type of file to be exported.
		*	\since version 0.0.2
		*/
		virtual void to(std::string path, zFileTpye type) = 0;

		/*! \brief This method exports the object linked to JSON.
		*
		*	\param [in]		path			- output file name including the directory path and extension.
		*	\param [in]		type			- type of file to be exported.
		*	\since version 0.0.2
		*/
		virtual void to(json &j) = 0;

		/*! \brief This method gets the JSON file from the input path if it exists.
		*
		*	\param		[in]	path			- input file path.
		*	\param		[out]	j				- output JSON file if it exists.
		*	\return 			bool			- true if file exists, else false.
		*	\since version 0.0.4
		*/
		bool json_read(string path, json& j);

		/*! \brief This method writes the JSON file to the output path.
		*
		*	\param		[out]	path			- output file path.
		*	\param		[in]	j				- output JSON file if it exists.
		*	\return 			bool			- true if file exists, else false.
		*	\since version 0.0.4
		*/
		bool json_write(string path, json& j);

		/*! \brief This method gets the attributes from the input json.
		*
		*	\param		[in]	j				- input JSON file if it exists.
		*	\param		[in]	attributeKey	- input JSON attribute name.
		*	\param		[in]	outAttribute	- input JSON attribute values.
		*	\return 			bool			- true if file exists, else false.
		*	\since version 0.0.4
		*/
		template <typename T>
		bool json_readAttribute(json& j, std::string attributeKey, T& outAttribute);

		/*! \brief This method gets the JSON file from the input path if it exists.
		*
		*	\param		[in]	path			- input file path.
		*	\param		[out]	j				- output JSON file if it exists.
		*	\return 			bool			- true if file exists, else false.
		*	\since version 0.0.4
		*/
		template <typename T>
		void json_writeAttribute(json& j, std::string attributeKey, T& outAttribute);

#if defined ZSPACE_USD_INTEROP

		/*! \brief This method imports the object linked to function type.
		*
		*	\param	[in]	usd				- input usd.
		*	\param	[in]	staticGeom		- true if the object is static. Helps speed up display especially for meshes object. Default set to false.
		*	\since version 0.0.2
		*/
		virtual void from(UsdPrim& usd, bool staticGeom = false) = 0;

		/*! \brief This method exports the object linked to USD.
		*
		*	\param [in]		path			- output file name including the directory path and extension.
		*	\param [in]		type			- type of file to be exported.
		*	\since version 0.0.2
		*/
		virtual void to(UsdPrim& usd) = 0;

		/*! \brief This method returns the stage pointer if it exists.
		*
		*	\param [in]		path			-  file name including the directory path and extension.
		*	\param [out]	uStage			-  usd stage pointer if it exists.
		*	\return 		bool			- true if file exists, else false.
		*	\since version 0.0.4
		*/
		bool usd_openStage(std::string path, UsdStageRefPtr& uStage);

		/*! \brief This method creates a new stage in the specified path.
		*
		*	\param [in]		path			- file name including the directory path and extension.
		*	\param [out]	uStage			- usd stage pointer if it exists.
		*	\return 		bool			- true if file exists, else false.
		*	\since version 0.0.4
		*/
		bool usd_createStage(std::string path, UsdStageRefPtr& uStage);

		/*! \brief This method creates a new geometry prim with the input name & stage.
		*
		*	\param [in]		uStage				- usd stage pointer.
		*	\param [in]		s_prim				- name of the geometry prim.
		*	\param [out]	geomPrim			- output geometry prim.
		*	\return 		bool				- true if file exists, else false.
		*	\since version 0.0.4
		*/
		template <typename T>
		void usd_createGeomPrim(UsdStageRefPtr& uStage, string s_prim, T& geomPrim);
		

#endif
		/*! \brief This method clears the dynamic and array memory the object holds.
		*
		*	\param [out]		minBB			- output minimum bounding box.
		*	\param [out]		maxBB			- output maximum bounding box.
		*	\since version 0.0.2
		*/
		virtual void getBounds(zPoint &minBB, zPoint &maxBB) = 0;

		/*! \brief This method clears the dynamic and array memory the object holds.
		*
		*	\since version 0.0.2
		*/
		virtual void clear() = 0;


		//--------------------------
		//---- SET METHODS
		//--------------------------

		/*! \brief This method sets the object transform to the input transform.
		*
		*	\param [in]		inTransform			- input transform.
		*	\param [in]		decompose			- decomposes transform to rotation and translation if true.
		*	\param [in]		updatePositions		- updates the object positions if true.
		*	\since version 0.0.2
		*/
		virtual void setTransform(zTransform &inTransform, bool decompose = true, bool updatePositions = true) = 0;
	
		
		/*! \brief This method sets the scale components of the object.
		*
		*	\param [in]		scale		- input scale values.
		*	\since version 0.0.2
		*/
		virtual void setScale(zFloat4 &scale) = 0;
	

		/*! \brief This method sets the rotation components of the object.
		*
		*	\param [in]		rotation			- input rotation values.
		*	\param [in]		append		- true if the input values are added to the existing rotations.
		*	\since version 0.0.2
		*/
		virtual void setRotation(zFloat4 &rotation, bool append = false) = 0;
	

		/*! \brief This method sets the translation components of the object.
		*
		*	\param [in]		translation			- input translation vector.
		*	\param [in]		append				- true if the input values are added to the existing translation.
		*	\since version 0.0.2
		*/
		virtual void setTranslation(zVector &translation, bool append = false) = 0;
		

		/*! \brief This method sets the pivot of the object.
		*
		*	\param [in]		pivot				- input pivot position.
		*	\since version 0.0.2
		*/
		virtual void setPivot(zVector &pivot) = 0;
	
		//--------------------------
		//---- GET METHODS
		//--------------------------

		/*! \brief This method gets the object transform.
		*
		*	\since version 0.0.2
		*/
		virtual  void getTransform(zTransform &transform) = 0;

		//--------------------------
		//---- TRANSFORMATION METHODS
		//--------------------------
				
	protected:		

		/*! \brief This method scales the object with the input scale transformation matix.
		*
		*	\since version 0.0.2
		*/
		virtual void transformObject(zTransform &transform) = 0;

	};


#ifndef DOXYGEN_SHOULD_SKIP_THIS
	//--------------------------
	//---- TEMPLATE METHODS INLINE DEFINITIONS
	//--------------------------

	template<typename T>
	inline bool zFn::json_readAttribute(json& j, std::string attributeKey, T& outAttribute)
	{		
		bool out = j.contains(attributeKey);
		if (out) outAttribute = j[attributeKey].get<T>();
		return out;
	}

	template<typename T>
	inline void zFn::json_writeAttribute(json& j, std::string attributeKey, T& outAttribute)
	{
		j[attributeKey] = outAttribute;
	}

#if defined ZSPACE_USD_INTEROP

	//---- UsdGeomMesh specilization for usd_createGeomPrim
	template<>
	inline void zFn::usd_createGeomPrim(UsdStageRefPtr& uStage, string s_prim, UsdGeomMesh& geomPrim)
	{
		geomPrim = UsdGeomMesh::Define(uStage, SdfPath("/World/Geometry/" + s_prim));

		if(!geomPrim) cout << " error creating UsdGeomMesh at  /World/Geometry/" << s_prim.c_str() << endl;
	}


#endif

#endif
}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterface/functionsets/zFn.cpp>
#endif

#endif