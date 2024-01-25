// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2022 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>, Heba Eiz <heba.eiz@zaha-hadid.com>
//



#include <headers/zInterface/functionsets/zFnComputeMesh.h>

namespace  zSpace
{

	//---- CONSTRUCTOR
	ZSPACE_INLINE zFnComputeMesh::zFnComputeMesh()
	{
		fnType = zFnType::zComputeMeshFn;
		compMeshObj = nullptr;
	}

	ZSPACE_INLINE zFnComputeMesh::zFnComputeMesh(zObjComputeMesh& _computeMeshObj)
	{
		fnType = zFnType::zComputeMeshFn;
		compMeshObj = &_computeMeshObj;
	};

	//---- DESTRUCTOR

	ZSPACE_INLINE zFnComputeMesh::~zFnComputeMesh() {}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zFnComputeMesh::getBounds(zPoint& minBB, zPoint& maxBB) { }

	ZSPACE_INLINE zFnType zFnComputeMesh::getType() 
	{
		return zComputeMeshFn;
	}

	ZSPACE_INLINE void zFnComputeMesh::from(string path, zFileTpye type, bool staticGeom) {}

	ZSPACE_INLINE void zFnComputeMesh::to(string path, zFileTpye type) {}

	ZSPACE_INLINE void zFnComputeMesh::clear() {}

	//----- CREATE METHODS

	ZSPACE_INLINE bool zFnComputeMesh::create(double* _vertexPositions, int* _polyCounts, int* _polyConnects, int* _triCounts, int* _triConnects, int numVerts, int numFaces)
	{
		if (!_vertexPositions || !_polyCounts || !_polyConnects) throw std::invalid_argument(" error: mesh container is empty.");
		compMeshObj = new zObjComputeMesh();
		//set all counts
		compMeshObj->nV = numVerts;
		compMeshObj->nF = numFaces;
		compMeshObj->polyCounts = _polyCounts;
		compMeshObj->triCounts = _triCounts;
		compMeshObj->dT = 0.5;

		compMeshObj->vFixed = new bool[compMeshObj->nV];
		for (int i = 0; i < compMeshObj->nV; i++)
		{
			compMeshObj->vFixed[i] = false;
		}


		for (int i = 0; i < numVerts; i += 1)
		{
			compMeshObj->vertexPositions[i].x = _vertexPositions[i * 3 + 0];
			compMeshObj->vertexPositions[i].y = _vertexPositions[i * 3 + 1];
			compMeshObj->vertexPositions[i].z = _vertexPositions[i * 3 + 2];

		}
		
		setPolygon(_polyConnects);

		setEdges();

		setParticles();
		
		compMeshObj->isValid = true;
	}

	ZSPACE_INLINE void zFnComputeMesh::setTriangles(int* _triConnects)
	{
		if (!compMeshObj || !compMeshObj->triCounts || !_triConnects) throw std::invalid_argument(" error: mesh container is empty.");

		try
		{
			compMeshObj->triangles = new int* [compMeshObj->nF];
			int triconnectsCurrentIndex = 0;
			for (int i = 0; i < compMeshObj->nF; i++)
			{
				compMeshObj->nT += compMeshObj->triCounts[i];
				int num_triVerts = compMeshObj->triCounts[i];
				compMeshObj->triangles[i] = new int[compMeshObj->triCounts[i]];

				for (int j = 0; j < num_triVerts * 3; j++)
				{
					compMeshObj->triangles[i][j] = _triConnects[triconnectsCurrentIndex];
					triconnectsCurrentIndex++;
				}
			}
		}
		catch (const std::exception&)
		{
			throw std::invalid_argument(" error: setTriangles is empty.");

		}

		

	}

	ZSPACE_INLINE void zFnComputeMesh::setCVertices(int* _vertexIndices, int* _cvCounts)
	{
		if (!_vertexIndices || !_vertexIndices || !compMeshObj) throw std::invalid_argument(" error: triangles container is empty.");
		compMeshObj->cVertices = new int* [compMeshObj->nV];
		compMeshObj->cVerticesCounts = new int [compMeshObj->nV];

		int currentIndex = 0;
		for (int i = 0; i < compMeshObj->nV; i++)
		{
			compMeshObj->cVertices[i] = new int[_cvCounts[i]];
			compMeshObj->cVerticesCounts[i] = _cvCounts[i];
			for (int j = 0; j < _cvCounts[i]; j++)
			{
				compMeshObj->cVertices[i][j] = _vertexIndices[currentIndex];
				currentIndex++;
			}
		}

	}

	ZSPACE_INLINE MatrixXd zFnComputeMesh::setMatrixV()
	{
		if (!compMeshObj || !compMeshObj->isValid) throw std::invalid_argument(" error: mesh container is empty.");
		try
		{
			MatrixXd V(compMeshObj->nV, 3);
			// fill vertex matrix
			for (int i = 0; i < compMeshObj->nV; i++)
			{
				V(i, 0) = compMeshObj->vertexPositions[i].x;
				V(i, 1) = compMeshObj->vertexPositions[i].y;
				V(i, 2) = compMeshObj->vertexPositions[i].z;
			}

			return V;
		}
		catch (const std::exception&)
		{
			throw std::invalid_argument(" error: setMatrixV error.");
		}

	}

	ZSPACE_INLINE MatrixXi zFnComputeMesh::setMatrixFTris()
	{
		if (!compMeshObj || !compMeshObj->isValid || !compMeshObj->triangles) throw std::invalid_argument(" error: mesh container is invalid.");
		try
		{
			MatrixXi FTris(compMeshObj->nT, 3);

			int nTris = 0;
			for (int i = 0; i < compMeshObj->nT; i++)
			{
				for (int j = 0; j < compMeshObj->triCounts[i]; j += 3)
				{
					FTris(nTris, 0) = compMeshObj->triangles[i][j + 0];
					FTris(nTris, 1) = compMeshObj->triangles[i][j + 1];
					FTris(nTris, 2) = compMeshObj->triangles[i][j + 2];

					nTris++;
				}
			}

			return FTris;
		}
		catch (const std::exception&)
		{
			throw std::invalid_argument(" error: setMatrixFTris error");
		}

	}
	
	
	//----  CURVATURE METHODS

	ZSPACE_INLINE void zFnComputeMesh::computeCurvatureDirections(VectorXd& PV1, VectorXd& PV2, MatrixXd& PD1, MatrixXd& PD2)
	{
		////calculate MatrixV and MatrixFTris
		//MatrixXd V = setMatrixV();
		//MatrixXi FTris = setMatrixFTris();
		//igl::principal_curvature(V, FTris, PD1, PD2, PV1, PV2);

	}

	ZSPACE_INLINE void zFnComputeMesh::computeGaussianCurvature(VectorXd& K)
	{
		////calculate MatrixV and MatrixFTris
		//MatrixXd V = setMatrixV();
		//MatrixXi FTris = setMatrixFTris();

		//// Compute integral of Gaussian curvature
		//igl::gaussian_curvature(V, FTris, K);
		//// Compute mass matrix
		//SparseMatrix<double> M, Minv;
		//igl::massmatrix(V, FTris, igl::MASSMATRIX_TYPE_DEFAULT, M);
		//igl::invert_diag(M, Minv);
		//// Divide by area to get integral average
		//K = (Minv * K).eval();

	}
	
	ZSPACE_INLINE void zFnComputeMesh::computeMeanCurvature(VectorXd& HV, MatrixXd& HN)
	{

		////calculate MatrixV and MatrixFTris
		//MatrixXd V = setMatrixV();
		//MatrixXi FTris = setMatrixFTris();

		//// Alternative discrete mean curvature
		//SparseMatrix<double> L, M, Minv;
		//igl::cotmatrix(V, FTris, L);
		//igl::massmatrix(V, FTris, igl::MASSMATRIX_TYPE_VORONOI, M);
		//igl::invert_diag(M, Minv);

		//// Laplace-Beltrami of position
		//HN = -Minv * (L * V);

		//// Extract magnitude as mean curvature
		//HV = HN.rowwise().norm();

	}

	
	//----  COMPUTE METHODS

	ZSPACE_INLINE void zFnComputeMesh::computeFaceVolumes(zPointArray& fCenters, zDoubleArray& fVolumes)
	{
		if(! compMeshObj) throw std::invalid_argument(" error: mesh container is invalid.");
		zUtilsCore core;

		fVolumes.clear();
		fVolumes.assign(compMeshObj->nF, float());

		fCenters.clear();
		fCenters.assign(compMeshObj->nF, zPoint());

		for (int i = 0; i < compMeshObj->nF; i++)
		{
			// compute face center
			for (int j = 0; j < compMeshObj->polyCounts[i]; j++) fCenters[i] += compMeshObj->vertexPositions[compMeshObj->polygons[i][j]];
			fCenters[i] /= compMeshObj->polyCounts[i];

			int nTris = floor(compMeshObj->triCounts[i] / 3);

			// add volume of face tris	
			for (int j = 0; j < compMeshObj->triCounts[i]; j += 3)
			{
				double vol = core.getSignedTriangleVolume(compMeshObj->vertexPositions[compMeshObj->triangles[i][j + 0]], compMeshObj->vertexPositions[compMeshObj->triangles[i][j + 1]], compMeshObj->vertexPositions[compMeshObj->triangles[i][j + 2]]);
				fVolumes[i] += vol;
			}

			// add volumes of tris formed by each pair of face edge vertices and face center
			for (int j = 0; j < compMeshObj->polyCounts[i]; j++)
			{
				int prevId = (j - 1 + compMeshObj->polyCounts[i]) % compMeshObj->polyCounts[i];

				double vol = core.getSignedTriangleVolume(compMeshObj->vertexPositions[compMeshObj->polygons[i][j]], compMeshObj->vertexPositions[compMeshObj->polygons[i][prevId]], fCenters[i]);
				fVolumes[i] += vol;
			}

			//absolute value
			fVolumes[i] = abs(fVolumes[i]);
		}
	}

	ZSPACE_INLINE void zFnComputeMesh::computeFaceNormals(zVectorArray& fNormals)
	{
		if (compMeshObj->nT == 0) throw std::invalid_argument(" error: triangles container is empty.");

		fNormals.clear();
		fNormals.assign(compMeshObj->nF, zVector());

		for (int i = 0; i < compMeshObj->nF; i++)
		{
			int nTris = floor(compMeshObj->triCounts[i] / 3);

			for (int j = 0; j < compMeshObj->triCounts[i]; j += 3)
			{
				zVector cross = (compMeshObj->vertexPositions[compMeshObj->triangles[i][j + 1]] - compMeshObj->vertexPositions[compMeshObj->triangles[i][j + 0]]) ^ (compMeshObj->vertexPositions[compMeshObj->triangles[i][j + 2]] - compMeshObj->vertexPositions[compMeshObj->triangles[i][j + 0]]);
				cross.normalize();

				fNormals[i] += cross;
			}

			fNormals[i] /= nTris;
			fNormals[i].normalize();
		}
	}

	ZSPACE_INLINE void zFnComputeMesh::computeQuadPlanarityDeviation(zDoubleArray& fDeviations)
	{
		if (!compMeshObj || !compMeshObj->nV == 0) throw std::invalid_argument(" error: compute mesh containers are empty.");

		zUtilsCore core;

		fDeviations.clear();
		fDeviations.assign(compMeshObj->nF, double());

		for (int j = 0; j < compMeshObj->nF; j++)
		{
			double uA, uB;
			zPoint pA, pB;

			bool check = core.line_lineClosestPoints(compMeshObj->vertexPositions[compMeshObj->polygons[j][0]], compMeshObj->vertexPositions[compMeshObj->polygons[j][2]], compMeshObj->vertexPositions[compMeshObj->polygons[j][1]], compMeshObj->vertexPositions[compMeshObj->polygons[j][3]], uA, uB, pA, pB);

			fDeviations[j] = pA.distanceTo(pB);
		}

	}

	ZSPACE_INLINE void zFnComputeMesh::computeBounds(zDomainVector& bounds)
	{
		bounds.min.x = bounds.min.y = bounds.min.z = std::numeric_limits<float>::max();
		bounds.max.x = bounds.max.y = bounds.max.z = std::numeric_limits<float>::min();

		for (int i = 0; i < compMeshObj->nV; i++)
		{
			zPoint p = compMeshObj->vertexPositions[i];
			if (p.x < bounds.min.x) bounds.min.x = p.x;
			if (p.y < bounds.min.y) bounds.min.y = p.y;
			if (p.z < bounds.min.z) bounds.min.z = p.z;

			if (p.x > bounds.max.x) bounds.max.x = p.x;
			if (p.y > bounds.max.y) bounds.max.y = p.y;
			if (p.z > bounds.max.z) bounds.max.z = p.z;
		}
	}


	//---- TRANSFORM METHODS OVERRIDES

	ZSPACE_INLINE void zFnComputeMesh::setTransform(zTransform& inTransform, bool decompose, bool updatePositions)
	{
	}

	ZSPACE_INLINE void zFnComputeMesh::setScale(zFloat4& scale)
	{
	}

	ZSPACE_INLINE void zFnComputeMesh::setRotation(zFloat4& rotation, bool appendRotations)
	{
	}

	ZSPACE_INLINE void zFnComputeMesh::setTranslation(zVector& translation, bool appendTranslations)
	{
	}

	ZSPACE_INLINE void zFnComputeMesh::setPivot(zVector& pivot)
	{
	}

	ZSPACE_INLINE void zFnComputeMesh::getTransform(zTransform& transform)
	{
	}


	//----- PROTECTED METHODS

	ZSPACE_INLINE void zFnComputeMesh::transformObject(zTransform& transform)
	{
	}

	ZSPACE_INLINE void zFnComputeMesh::setPolygon(int* _polyConnects)
	{
		if (!compMeshObj || compMeshObj->nF == 0 || !compMeshObj->polyCounts) throw std::invalid_argument(" error: mesh container is empty.");
		try
		{
			compMeshObj->polygons = new int* [compMeshObj->nF];

			int polyconnectsIndex = 0;
			for (int i = 0; i < compMeshObj->nF; i++)
			{
				compMeshObj->polygons[i] = new int[compMeshObj->polyCounts[i]];
				for (int j = 0; j < compMeshObj->polyCounts[i]; j++)
				{
					compMeshObj->polygons[i][j] = _polyConnects[polyconnectsIndex];
					polyconnectsIndex++;
				}
			}
		}
		catch (const std::exception&)
		{
			throw std::invalid_argument(" error: mesh container is empty.");
		}

	}

	ZSPACE_INLINE void zFnComputeMesh::setEdges()// int* _polyCounts, int** polygons, int numFaces, int& nE)
	{

		if (!compMeshObj || compMeshObj->nF == 0 || !compMeshObj->polyCounts) throw std::invalid_argument(" error: mesh container is empty.");
		try
		{
			compMeshObj->nE = 0;
			//	halfedge connectivity map
			unordered_map <string, int> existingHalfEdges;
			zInt2DArray edges; //temporary

			for (int i = 0; i < compMeshObj->nF; i++)
			{
				for (int j = 0; j < compMeshObj->polyCounts[i]; j++)
				{
					int v1 = compMeshObj->polygons[i][j];

					int next = (j + 1) % compMeshObj->polyCounts[i];
					int v2 = compMeshObj->polygons[i][next];

					string e1 = (to_string(v1) + "," + to_string(v2));
					std::unordered_map<std::string, int>::const_iterator got = existingHalfEdges.find(e1);

					if (got != existingHalfEdges.end())
					{
						// edge exists continue
					}
					else
					{
						// add edge
						edges.push_back(zIntArray());
						edges[compMeshObj->nE].push_back(v1);
						edges[compMeshObj->nE].push_back(v2);

						// add halfedges to map
						existingHalfEdges[e1] = compMeshObj->nE * 2 + 0;

						string e2 = (to_string(v2) + "," + to_string(v1));
						existingHalfEdges[e2] = compMeshObj->nE * 2 + 1;

						compMeshObj->nE++;

					}
				}
			}

			//assign edges vector to 2dArray
			compMeshObj->edges = new int* [compMeshObj->nE];
			for (int i = 0; i < compMeshObj->nE; i++)
			{
				compMeshObj->edges[i] = new int[2];
				compMeshObj->edges[i][0] = edges[i][0];
				compMeshObj->edges[i][1] = edges[i][1];
			}
		}
		catch (const std::exception&)
		{
			throw std::invalid_argument(" error: set edges error.");
		}
	}

	ZSPACE_INLINE void zFnComputeMesh::setParticles()//, int numVerts,  zPoint* vertexPositions, zObjParticle* o_Particles, zFnParticle* fn_Particles, zPoint* vertexPositions)
	{

		compMeshObj->o_Particles = new zObjParticle[compMeshObj->nV];
		//compMeshObj->fnParticles = new zFnParticle[compMeshObj->nV];

		for (int i = 0; i < compMeshObj->nV; i++)
		{
			bool fixed = false;
			zObjParticle p;
			p.particle = zParticle(compMeshObj->vertexPositions[i], fixed);
			compMeshObj->o_Particles[i] = p;
			//compMeshObj->fnParticles[i] = zFnParticle(compMeshObj->o_Particles[i]);

		}
	}

}

