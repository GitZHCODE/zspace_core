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


#include "zCore/geometry/zMesh.h"

namespace zSpace
{

	//---- CONSTRUCTOR
	
	ZSPACE_INLINE zMesh::zMesh()
	{
		n_v = n_e = n_he = n_f = 0;

	}
	
	//---- DESTRUCTOR

	ZSPACE_INLINE zMesh::~zMesh()
	{

	}

	//---- CREATE METHODS

	ZSPACE_INLINE void zMesh::create(zPointArray(&_positions), zIntArray(&polyCounts), zIntArray(&polyConnects))
	{
		//clear containers
		clear();

		int num_edges = computeNumEdges(polyCounts, polyConnects);

		//vertices.reserve(_positions.size() /*+ 1*/);
		//faces.reserve(polyCounts.size() /*+ 1*/);
		//edges.reserve(num_edges/* + 1*/);
		//halfEdges.reserve(num_edges * 2 /*+ 2*/);

		// create vertices

		for (int i = 0; i < _positions.size(); i++) addVertex(_positions[i]);


		// create faces and edges connection
		int polyconnectsCurrentIndex = 0;

		zIntArray fVerts;
		for (int i = 0; i < polyCounts.size(); i++)
		{
			int num_faceVerts = polyCounts[i];

			fVerts.clear();
			for (int j = 0; j < num_faceVerts; j++)
			{
				fVerts.push_back(polyConnects[polyconnectsCurrentIndex + j]);
			}

			addPolygon(fVerts);
			polyconnectsCurrentIndex += num_faceVerts;

		}


		// update boundary pointers
		update_BoundaryEdgePointers();

	}

	ZSPACE_INLINE void zMesh::clear()
	{
		vertices.clear();
		vertexPositions.clear();
		vertexNormals.clear();
		vertexColors.clear();
		vertexWeights.clear();
		positionVertex.clear();

		edges.clear();
		edgeColors.clear();
		edgeWeights.clear();


		halfEdges.clear();
		existingHalfEdges.clear();

		faces.clear();
		faceColors.clear();
		faceNormals.clear();

		vHandles.clear();
		eHandles.clear();
		heHandles.clear();
		fHandles.clear();

		n_v = n_e = n_he = n_f = 0;
	}

	//---- VERTEX METHODS

	ZSPACE_INLINE bool zMesh::addVertex(zVector &pos)
	{
		bool out = false;

		//if (n_v >= vertices.capacity() - 1)
		//{
		//	if (n_v > 0) resizeArray(zVertexData, n_v * 4);
		//	else resizeArray(zVertexData, 100);
		//	out = true;
		//}


		addToPositionMap(pos, n_v);

		//zItVertex newV = vertices.insert(vertices.end(), zVertex());
		vertices.push_back(zVertex());
		vertices[n_v].setId(n_v);


		vertexPositions.push_back(pos);

		vHandles.push_back(zVertexHandle());
		vHandles[n_v].id = n_v;

		n_v++;

		// default Attibute values			
		vertexColors.push_back(zColor(1, 0, 0, 1));
		vertexWeights.push_back(2.0);

		return out;
	}

	//---- EDGE METHODS

	ZSPACE_INLINE int zMesh::computeNumEdges(zIntArray(&polyCounts), zIntArray(&polyConnects))
	{
		set< pair<int, int> > tempEdges;

		int polyconnectsCurrentIndex = 0;

		zIntArray fVerts;
		for (int i = 0; i < polyCounts.size(); i++)
		{
			int num_faceVerts = polyCounts[i];


			for (int j = 0; j < num_faceVerts; j++)
			{
				int v0 = polyConnects[polyconnectsCurrentIndex + j];

				int next = (j + 1) % num_faceVerts;

				int v1 = polyConnects[polyconnectsCurrentIndex + next];

				if (v0 > v1) swap(v0, v1);

				tempEdges.insert(pair<int, int>(v0, v1));
			}


			polyconnectsCurrentIndex += num_faceVerts;

		}

		return tempEdges.size();
	}

	ZSPACE_INLINE bool zMesh::addEdges(int &v1, int &v2)
	{

		bool out = false;

		/*if (n_e >= edges.capacity() - 1)
		{

			if (n_e > 0)	resizeArray(zEdgeData, n_e * 4);
			else	resizeArray(zEdgeData, 100);
			out = true;
		}

		if (n_he >= halfEdges.capacity() - 2)
		{
			if (n_he > 0)resizeArray(zHalfEdgeData, n_he * 4);
			else resizeArray(zHalfEdgeData, 200);
			out = true;
		}*/


		// Handles
		//heHandles.push_back(zHalfEdgeHandle());
		//heHandles.push_back(zHalfEdgeHandle());
		//eHandles.push_back(zEdgeHandle());

		// HALF edge	
		//zItHalfEdge newHE1 = halfEdges.insert(halfEdges.end(), zHalfEdge());
		halfEdges.push_back(zHalfEdge());
		halfEdges[n_he].setId(n_he);
		halfEdges[n_he].setVertex(v2);
		halfEdges[n_he].setEdge(n_e);
		halfEdges[n_he].setSym(n_he + 1);
		
		//newHE1->setId(n_he);
		//newHE1->setVertex(&vertices[v2]);

		heHandles.push_back(zHalfEdgeHandle());
		heHandles[n_he].id = n_he;
		heHandles[n_he].v = v2;
		heHandles[n_he].e = n_e;
		
		n_he++;

		// SYMMETRY edge	
		//zItHalfEdge newHE2 = halfEdges.insert(halfEdges.end(), zHalfEdge());
		halfEdges.push_back(zHalfEdge());
		halfEdges[n_he].setId(n_he);
		halfEdges[n_he].setVertex(v1);
		halfEdges[n_he].setEdge(n_e);
		halfEdges[n_he].setSym(n_he - 1);
		//newHE2->setId(n_he);
		//newHE2->setVertex(&vertices[v1]);

		heHandles.push_back(zHalfEdgeHandle());
		heHandles[n_he].id = n_he;
		heHandles[n_he].v = v1;
		heHandles[n_he].e = n_e;

		n_he++;

		//EDGE
		//zItEdge newE = edges.insert(edges.end(), zEdge());
		edges.push_back(zEdge());
		edges[n_e].setId(n_e);
		edges[n_e].setHalfEdge(n_he - 2, 0);
		edges[n_e].setHalfEdge(n_he - 1, 1);
		//newE->setId(n_e);
		//newE->setHalfEdge(&halfEdges[n_he - 2], 0);
		//newE->setHalfEdge(&halfEdges[n_he - 1], 1);;


		//newHE1->setEdge(&edges[n_e]);
		//newHE2->setEdge(&edges[n_e]);

		eHandles.push_back(zEdgeHandle());
		eHandles[n_e].id = n_e;
		eHandles[n_e].he0 = n_he - 2;
		eHandles[n_e].he1 = n_he - 1;

		n_e++;

		// set symmetry iterators 
		//newHE2->setSym(&halfEdges[n_he - 2]);

		addToHalfEdgesMap(v1, v2, n_he - 2);


		// default color and weights
		edgeColors.push_back(zColor(0, 0, 0, 0));
		edgeWeights.push_back(1.0);

		return out;
	}

	ZSPACE_INLINE void zMesh::setStaticEdgeVertices(vector<zIntArray> &_edgeVertices)
	{
		if (!staticGeometry) 	throw std::invalid_argument(" error: mesh not static");
		edgeVertices = _edgeVertices;
	}

	//---- FACE METHODS

	ZSPACE_INLINE bool zMesh::addPolygon()
	{
		bool out = false;

		/*if (n_f >= faces.capacity() - 1)
		{
			if (n_f > 0) resizeArray(zFaceData, n_f * 4);
			else resizeArray(zFaceData, 100);

			out = true;
		}*/

		//zItFace newF = faces.insert(faces.end(), zFace());
		faces.push_back(zFace());
		faces[n_f].setId(n_f);
		//newF->setId(n_f);

		fHandles.push_back(zFaceHandle());
		fHandles[n_f].id = n_f;

		n_f++;



		//add default faceColors 
		faceColors.push_back(zColor(0.5, 0.5, 0.5, 1));

		return out;
	}

	ZSPACE_INLINE bool zMesh::addPolygon(zIntArray &fVertices)
	{
		// add null polygon
		bool out = addPolygon();

		// get edgeIds of face
		zIntArray fEdge;

		int currentNumEdges = n_he;
	

		for (int i = 0; i < fVertices.size(); i++)
		{
			// check if edge exists
			int eID;
			bool chkEdge = halfEdgeExists(fVertices[i], fVertices[(i + 1) % fVertices.size()], eID);

		
			if (chkEdge)
			{
				fEdge.push_back(eID);
			}
			else
			{
				addEdges(fVertices[i], fVertices[(i + 1) % fVertices.size()]);

				fEdge.push_back(n_he - 2);

			}

			//printf("\n %i %i %s %i", fVertices[i], fVertices[(i + 1) % fVertices.size()], (chkEdge) ? "true" : "false", fEdge[i]);
		}

		//update current face verts edge pointer
		for (int i = 0; i < fVertices.size(); i++)
		{
			vertices[fVertices[i]].setHalfEdge(fEdge[i]);

			vHandles[fVertices[i]].he = fEdge[i];
		}

		// update face, next and prev edge pointers for face Edges
		for (int i = 0; i < fEdge.size(); i++)
		{
			halfEdges[fEdge[i]].setFace(n_f - 1);

			halfEdges[fEdge[i]].setNext(fEdge[(i + 1) % fEdge.size()]);
			halfEdges[fEdge[i]].setPrev(fEdge[(i - 1 + fEdge.size()) % fEdge.size()]);

			halfEdges[fEdge[(i + 1) % fEdge.size()]].setPrev(fEdge[i]);
			halfEdges[fEdge[(i - 1 + fEdge.size()) % fEdge.size()]].setNext(fEdge[i]);

			heHandles[fEdge[i]].f = n_f - 1;
			heHandles[fEdge[i]].n = fEdge[(i + 1) % fEdge.size()];
			heHandles[fEdge[i]].p = fEdge[(i - 1 + fEdge.size()) % fEdge.size()];

			heHandles[fEdge[(i + 1) % fEdge.size()]].p = fEdge[i];
			heHandles[fEdge[(i - 1 + fEdge.size()) % fEdge.size()]].n = fEdge[i];
		}

		// update curent face edge
		//zFace *f1 = &faces[n_f - 1];
		faces[n_f - 1].setHalfEdge(fEdge[0]);

		fHandles[n_f - 1].he = fEdge[0];

		return out;
	}

	ZSPACE_INLINE bool zMesh::updatePolygon(int faceID, zIntArray& fVertices)
	{
		
		// get edgeIds of face
		zIntArray fEdge;

		int currentNumEdges = n_he;


		for (int i = 0; i < fVertices.size(); i++)
		{
			// check if edge exists
			int eID;
			bool chkEdge = halfEdgeExists(fVertices[i], fVertices[(i + 1) % fVertices.size()], eID);


			if (chkEdge)
			{
				fEdge.push_back(eID);
			}
			else
			{
				addEdges(fVertices[i], fVertices[(i + 1) % fVertices.size()]);

				fEdge.push_back(n_he - 2);

			}

			//printf("\n %i %i %s %i", fVertices[i], fVertices[(i + 1) % fVertices.size()], (chkEdge) ? "true" : "false", fEdge[i]);
		}

		//update current face verts edge pointer
		for (int i = 0; i < fVertices.size(); i++)
		{
			vertices[fVertices[i]].setHalfEdge(fEdge[i]);

			vHandles[fVertices[i]].he = fEdge[i];
		}

		// update face, next and prev edge pointers for face Edges
		for (int i = 0; i < fEdge.size(); i++)
		{
			halfEdges[fEdge[i]].setFace(faceID);

			halfEdges[fEdge[i]].setNext(fEdge[(i + 1) % fEdge.size()]);
			halfEdges[fEdge[i]].setPrev(fEdge[(i - 1 + fEdge.size()) % fEdge.size()]);

			halfEdges[fEdge[(i + 1) % fEdge.size()]].setPrev(fEdge[i]);
			halfEdges[fEdge[(i - 1 + fEdge.size()) % fEdge.size()]].setNext(fEdge[i]);

			heHandles[fEdge[i]].f = faceID;
			heHandles[fEdge[i]].n = fEdge[(i + 1) % fEdge.size()];
			heHandles[fEdge[i]].p = fEdge[(i - 1 + fEdge.size()) % fEdge.size()];

			heHandles[fEdge[(i + 1) % fEdge.size()]].p = fEdge[i];
			heHandles[fEdge[(i - 1 + fEdge.size()) % fEdge.size()]].n = fEdge[i];
		}

		// update curent face edge
		//zFace *f1 = &faces[n_f - 1];
		faces[faceID].setHalfEdge(fEdge[0]);

		fHandles[faceID].he = fEdge[0];

		return false;
	}

	ZSPACE_INLINE void zMesh::setNumPolygons(int _n_f, bool setMax)
	{
		n_f = _n_f;

	}

	ZSPACE_INLINE void zMesh::getFaceEdges(int index, zIntArray &edgeIndicies)
	{

		edgeIndicies.clear();

		if (faces[index].getHalfEdge() != -1)
		{
			zHalfEdge start = halfEdges[faces[index].getHalfEdge()];
			zHalfEdge e = start;

			bool exit = false;			
			do
			{
				edgeIndicies.push_back(e.getId());
				if (e.getNext()!= -1)e = halfEdges[e.getNext()];
				else exit = true;

			} while (e.getId() != start.getId() && !exit);

			
		}


	}

	ZSPACE_INLINE void zMesh::getFaceVertices(int index, zIntArray &vertexIndicies)
	{
		vertexIndicies.clear();

		zIntArray faceEdges;
		getFaceEdges(index, faceEdges);

		for (int i = 0; i < faceEdges.size(); i++)
		{
			zHalfEdge he = halfEdges[faceEdges[i]];
			zHalfEdge he_sym = halfEdges[he.getSym()];
			vertexIndicies.push_back(he_sym.getVertex());
		}

		

	}

	ZSPACE_INLINE void zMesh::setStaticFaceVertices(vector<zIntArray> &_faceVertices)
	{
		if (!staticGeometry) 	throw std::invalid_argument(" error: geometry not static");
		faceVertices = _faceVertices;
	}

	//---- UTILITY METHODS

	ZSPACE_INLINE void zMesh::indexElements(zHEData type)
	{
		if (type == zVertexData)
		{
			int n_v = 0;
			for (auto &v : vertices)
			{
				v.setId(n_v);
				n_v++;
			}
		}

		else if (type == zEdgeData)
		{
			int n_e = 0;
			for (auto &e : edges)
			{
				e.setId(n_e);
				n_e++;
			}
		}

		else if (type == zHalfEdgeData)
		{
			int n_he = 0;
			for (auto &he : halfEdges)
			{
				he.setId(n_he);
				n_he++;
			}
		}

		else if (type == zFaceData)
		{
			int n_f = 0;
			for (auto &f : faces)
			{
				f.setId(n_f);
				n_f++;
			}
		}

		else throw std::invalid_argument(" error: invalid zHEData type");
	}

	ZSPACE_INLINE void zMesh::update_BoundaryEdgePointers()
	{


		for (int i = 0; i < halfEdges.size(); i++)
		{



			if (halfEdges[i].getFace() == -1)
			{
				//printf("\n %i ", i);

				zHalfEdge e = halfEdges[halfEdges[i].getSym()];
				bool exit = false;

				do
				{
					e = halfEdges[e.getPrev()];
					e = halfEdges[e.getSym()];

				} while (e.getFace() != -1);

				halfEdges[i].setNext(e.getId());
				halfEdges[e.getId()].setPrev(i);

				heHandles[i].n = e.getId();
				heHandles[e.getId()].p = i;

				zHalfEdge e1 = halfEdges[halfEdges[i].getSym()];

				do
				{
					e1 = halfEdges[e1.getNext()];
					e1 = halfEdges[e1.getSym()];

				} while (e1.getFace()!= -1);

				halfEdges[i].setPrev(e1.getId());
				halfEdges[e1.getId()].setNext(i);

				heHandles[i].p = e1.getId();
				heHandles[e1.getId()].n = i;
			}




		}
	}

	ZSPACE_INLINE void zMesh::resizeArray(zHEData type, int newSize)
	{
		//  Vertex
		if (type == zVertexData)
		{
			vertices.clear();
			vertices.reserve(newSize);

			// reassign pointers
			int n_v = 0;
			for (auto &v : vHandles)
			{
				zItVertex newV = vertices.insert(vertices.end(), zVertex());
				newV->setId(n_v);
				if (v.he != -1)newV->setHalfEdge(v.he);

				n_v++;
			}

			for (int i = 0; i < heHandles.size(); i++)
			{
				if (heHandles[i].v != -1) halfEdges[i].setVertex(heHandles[i].v);
			}

			//printf("\n mesh vertices resized. ");

		}

		//  Edge
		else if (type == zHalfEdgeData)
		{

			halfEdges.clear();
			halfEdges.reserve(newSize);

			// reassign pointers
			int n_he = 0;
			for (auto &he : heHandles)
			{

				zItHalfEdge newHE = halfEdges.insert(halfEdges.end(), zHalfEdge());

				newHE->setId(n_he);
				if (he.n != -1) newHE->setNext(he.n);
				if (he.p != -1) newHE->setPrev(he.p);
				if (he.v != -1) newHE->setVertex(he.v);
				if (he.e != -1) newHE->setEdge(he.e);
				if (he.f != -1) newHE->setFace(he.f);


				if (n_he % 2 == 1) newHE->setSym(n_he - 1);
				else newHE->setSym(n_he + 1);

				n_he++;
			}

			for (int i = 0; i < vHandles.size(); i++)
			{
				if (vHandles[i].he != -1) vertices[i].setHalfEdge(vHandles[i].he);
			}

			for (int i = 0; i < eHandles.size(); i++)
			{
				if (eHandles[i].he0 != -1) edges[i].setHalfEdge(eHandles[i].he0, 0);
				if (eHandles[i].he1 != -1) edges[i].setHalfEdge(eHandles[i].he1, 1);
			
			}

			for (int i = 0; i < fHandles.size(); i++)
			{
				if (fHandles[i].he != -1) faces[i].setHalfEdge(fHandles[i].he);
			}



			//printf("\n mesh half edges resized. ");
		}

		else if (type == zEdgeData)
		{

		

			edges.clear();
			edges.reserve(newSize);

			// reassign pointers

			int n_e = 0;
			for (auto &e : eHandles)
			{
				zItEdge newE = edges.insert(edges.end(), zEdge());
				newE->setId(n_e);

				if (e.he0 != -1)newE->setHalfEdge(e.he0, 0);
				if (e.he1 != -1)newE->setHalfEdge(e.he1, 1);
				

				n_e++;

			}

			for (int i = 0; i < heHandles.size(); i++)
			{
				if (heHandles[i].e != -1) halfEdges[i].setEdge(heHandles[i].e);
			}

			//printf("\n mesh edges resized. ");	
		}

		// Mesh Face
		else if (type == zFaceData)
		{

			faces.clear();
			faces.reserve(newSize);

			// reassign pointers
			int n_f = 0;
			for (auto &f : fHandles)
			{
				zItFace newF = faces.insert(faces.end(), zFace());
				newF->setId(n_f);
				
				if (f.he != -1)newF->setHalfEdge(f.he);


				n_f++;

			}

			for (int i = 0; i < heHandles.size(); i++)
			{
				if (heHandles[i].f != -1) halfEdges[i].setFace(heHandles[i].f);
			}


			//printf("\n mesh faces resized. ");
		}

		else throw std::invalid_argument(" error: invalid zHEData type");
	}
}