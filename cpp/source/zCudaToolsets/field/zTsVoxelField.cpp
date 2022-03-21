#include <headers/zCudaToolsets/field/zTsVoxelField.h>

//-----
// zCdGraph
//-----
namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zCdGraph::zCdGraph() {}

	//---- DESTRUCTOR

	ZSPACE_INLINE zCdGraph::~zCdGraph() {}

	//---- SET METHODS

	ZSPACE_INLINE void zCdGraph::setGraph(const float* _vertexPositions, const int* _edgeConnects, int _numVertices, int _numEdges)
	{
		vertexPositions = new float[_numVertices * 3];
		std::copy(_vertexPositions, _vertexPositions + _numVertices, vertexPositions);
		nV = _numVertices;

		edgeConnects = new int[_numEdges * 2];
		std::copy(_edgeConnects, _edgeConnects + _numEdges, edgeConnects);
		nE = _numEdges;
	}

	//---- GET METHODS

	ZSPACE_INLINE int zCdGraph::numVertices()
	{
		return nV;
	}

	ZSPACE_INLINE int zCdGraph::numEdges()
	{
		return nE;
	}

	ZSPACE_INLINE float* zCdGraph::getRawVertexPositions()
	{
		return vertexPositions;
	}

	ZSPACE_INLINE int* zCdGraph::getRawEdgeConnects()
	{
		return edgeConnects;
	}

}


//-----
// zTsVoxelField
//-----
namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zTsVoxelField::zTsVoxelField() {}

	//---- DESTRUCTOR

	ZSPACE_INLINE zTsVoxelField::~zTsVoxelField() {}

	//---- SET METHODS


	ZSPACE_INLINE void zTsVoxelField::setVoxels(float* _voxelPositions, float* _voxelData, int _numVoxels)
	{
		voxelPositions = new float[_numVoxels * 3];;
		std::copy(_voxelPositions, _voxelPositions + (_numVoxels * 3), voxelPositions);

		voxelData = new float[_numVoxels];;
		std::copy(_voxelData, _voxelData + (_numVoxels * 1), voxelData);

		numVoxs = _numVoxels;
		setMemory(numVoxs * 4 + MAX_POINTCLOUD);


		int numPositions = numVoxs * 3;
		std::copy(voxelPositions, voxelPositions + numPositions, voxels);
		std::copy(voxelData, voxelData + numVoxs, voxels + (numVoxs * 3));
	}

	ZSPACE_INLINE void zTsVoxelField::setPointcloud(float* _pointCloud, int _numPoints)
	{
		pointCloud = new float[_numPoints];
		std::copy(_pointCloud, _pointCloud + (_numPoints * 1), pointCloud);
		numPoints = _numPoints;

		if (numPoints > MAX_POINTCLOUD)
		{
			setMemory((numVoxs * 4) + numPoints);

			std::copy(voxelPositions, voxelPositions + (numVoxs * 3), voxels);
			std::copy(voxelData, voxelData + numVoxs, voxels + (numVoxs * 3));
		}

		
		std::copy(pointCloud, pointCloud + numPoints, voxels + (numVoxs * 4));
	}

	ZSPACE_INLINE void zTsVoxelField::setDomain_Colors(zDomainColor& _dColor)
	{
		dColor = _dColor;
	}


	//---- GET METHODS

	ZSPACE_INLINE int zTsVoxelField::numVoxels()
	{
		return numVoxs;
	}

	ZSPACE_INLINE int zTsVoxelField::numCloudPoints()
	{
		return numPoints;
	}

	ZSPACE_INLINE int zTsVoxelField::getMemSize()
	{
		return memSize;
	}

	ZSPACE_INLINE float* zTsVoxelField::getRawVoxels()
	{
		return voxels;
	}

	ZSPACE_INLINE float* zTsVoxelField::getRawColors()
	{
		return voxelColors;
	}

	ZSPACE_INLINE zDomainColor zTsVoxelField::getDomain_Colors()
	{
		return dColor;
	}


	
	//---- COMPUTE METHODS

	ZSPACE_INLINE void zTsVoxelField::computeBin()
	{
		
	}


	//---- PROTECTED METHODS


	ZSPACE_INLINE void zTsVoxelField::setMemory(int nSize)
	{
		if ((nSize) < memSize) return;
		else
		{
			while (memSize < (nSize)) memSize += d_MEMORYMULTIPLIER;
			voxels = new float[memSize];	

			voxelColors = new float[memSize];
			
			printf("\n cpu  MemSize %i \n", memSize);
		}
	}


}
