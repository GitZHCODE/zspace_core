// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Authors : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com> , Leo Bieling <leo.bieling.zaha-hadid.com>
//


#include<zspace/zInterface/functionsets/zFnMeshField.h>

namespace zSpace
{

	//---- CONSTRUCTOR

	template<typename T>
	ZSPACE_INLINE zFnMeshFieldBase<T>::zFnMeshFieldBase()
	{
		fnType = zFnType::zMeshFieldFn;
		fieldObj = nullptr;

	}

	template<typename T>
	ZSPACE_INLINE zFnMeshFieldBase<T>::zFnMeshFieldBase(zObjectMeshField<T> &_fieldObj)
	{
		fieldObj = &_fieldObj;

		fnType = zFnType::zMeshFieldFn;
		fnMesh = zFnMesh(_fieldObj);

	}

	//---- DESTRUCTOR

	template<typename T>
	ZSPACE_INLINE zFnMeshFieldBase<T>::~zFnMeshFieldBase() {}

	//---- FACTORY METHODS

	template<typename T>
	ZSPACE_INLINE zFnType zFnMeshFieldBase<T>::getType()
	{
		return zFnType::zMeshFieldFn;
	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		minBB = fieldObj->field.minBB;
		maxBB = fieldObj->field.maxBB;
	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::clear()
	{
		ringNeighbours.clear();
		adjacentNeighbours.clear();
		fieldObj->field.fieldValues.clear();
		fnMesh.clear();
	}

	//---- CREATE METHODS

	//---- zScalar &  zVector specilization for create

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::create(zPoint _minBB, zPoint _maxBB, int _n_X, int _n_Y, int _NR, bool _setValuesperVertex, bool _triMesh)
	{
		setValuesperVertex = _setValuesperVertex;
		if (!_setValuesperVertex) _triMesh = false;
		triMesh = _triMesh;


		fieldObj->field = zField2D<zScalar>(_minBB, _maxBB, _n_X, _n_Y);
		fieldObj->field.valuesperVertex = setValuesperVertex;

		// compute neighbours
		ringNeighbours.clear();
		adjacentNeighbours.clear();


		ringNeighbours.assign(numFieldValues(), vector<int>());
		adjacentNeighbours.assign(numFieldValues(), vector<int>());

		int i = 0;
		for (zItMeshScalarField s(*fieldObj); !s.end(); s++, i++)
		{
			vector<int> temp_ringNeighbour;
			s.getNeighbour_Ring(_NR, temp_ringNeighbour);
			ringNeighbours[i] = (temp_ringNeighbour);

			vector<int> temp_adjacentNeighbour;
			s.getNeighbour_Adjacents(temp_adjacentNeighbour);
			adjacentNeighbours[i] = (temp_adjacentNeighbour);
		}

		createFieldMesh();

		updateColors();

	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zVector>::create(zPoint _minBB, zPoint _maxBB, int _n_X, int _n_Y, int _NR, bool _setValuesperVertex, bool _triMesh)
	{
		setValuesperVertex = _setValuesperVertex;
		if (!_setValuesperVertex) _triMesh = false;
		triMesh = _triMesh;


		fieldObj->field = zField2D<zVector>(_minBB, _maxBB, _n_X, _n_Y);
		fieldObj->field.valuesperVertex = setValuesperVertex;

		// compute neighbours
		ringNeighbours.clear();
		adjacentNeighbours.clear();


		ringNeighbours.assign(numFieldValues(), vector<int>());
		adjacentNeighbours.assign(numFieldValues(), vector<int>());

		int i = 0;
		for (zItMeshVectorField s(*fieldObj); !s.end(); s++, i++)
		{
			vector<int> temp_ringNeighbour;
			s.getNeighbour_Ring(_NR, temp_ringNeighbour);
			ringNeighbours[i] = (temp_ringNeighbour);

			vector<int> temp_adjacentNeighbour;
			s.getNeighbour_Adjacents(temp_adjacentNeighbour);
			adjacentNeighbours[i] = (temp_adjacentNeighbour);
		}

		createFieldMesh();		

	}

	//---- zScalar &  zVector specilization for create

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::create(double _unit_X, double _unit_Y, int _n_X, int _n_Y, zPoint _minBB, int _NR, bool _setValuesperVertex, bool _triMesh)
	{
		setValuesperVertex = _setValuesperVertex;
		if (!_setValuesperVertex) _triMesh = false;
		triMesh = _triMesh;

		fieldObj->field = zField2D<zScalar>(_unit_X, _unit_Y, _n_X, _n_Y, _minBB);
		fieldObj->field.valuesperVertex = setValuesperVertex;

		// compute neighbours
		ringNeighbours.clear();
		adjacentNeighbours.clear();

		ringNeighbours.assign(numFieldValues(), vector<int>());
		adjacentNeighbours.assign(numFieldValues(), vector<int>());

		int i = 0; 
		for( zItMeshScalarField s(*fieldObj); !s.end(); s++, i++)		
		{
			vector<int> temp_ringNeighbour;
			s.getNeighbour_Ring(_NR, temp_ringNeighbour);
			ringNeighbours[i] = (temp_ringNeighbour);

			vector<int> temp_adjacentNeighbour;
			s.getNeighbour_Adjacents(temp_adjacentNeighbour);
			adjacentNeighbours[i] = (temp_adjacentNeighbour);
		}

		createFieldMesh();

		updateColors();
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zVector>::create(double _unit_X, double _unit_Y, int _n_X, int _n_Y, zPoint _minBB, int _NR, bool _setValuesperVertex, bool _triMesh)
	{
		setValuesperVertex = _setValuesperVertex;
		if (!_setValuesperVertex) _triMesh = false;
		triMesh = _triMesh;

		fieldObj->field = zField2D<zVector>(_unit_X, _unit_Y, _n_X, _n_Y, _minBB);
		fieldObj->field.valuesperVertex = setValuesperVertex;

		// compute neighbours
		ringNeighbours.clear();
		adjacentNeighbours.clear();

		ringNeighbours.assign(numFieldValues(), vector<int>());
		adjacentNeighbours.assign(numFieldValues(), vector<int>());

		int i = 0;
		for (zItMeshVectorField s(*fieldObj); !s.end(); s++, i++)
		{
			vector<int> temp_ringNeighbour;
			s.getNeighbour_Ring(_NR, temp_ringNeighbour);
			ringNeighbours[i] = (temp_ringNeighbour);

			vector<int> temp_adjacentNeighbour;
			s.getNeighbour_Adjacents(temp_adjacentNeighbour);
			adjacentNeighbours[i] = (temp_adjacentNeighbour);
		}

		createFieldMesh();		
	}

	//---- zVector specilization for createVectorFieldFromScalarField

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zVector>::createVectorFromScalarField(zObjectMeshField<zScalar> &scalarFieldObj)
	{
		zFnMeshFieldBase<zScalar> fnScalarField(scalarFieldObj);

		zVector minBB, maxBB;
		fnScalarField.getBoundingBox(minBB, maxBB);

		int n_X, n_Y;
		fnScalarField.getResolution(n_X, n_Y);

		vector<zVector> gradients = fnScalarField.getGradients();

		create(minBB, maxBB, n_X, n_Y, fnScalarField.getValuesPerVertexBoolean(), fnScalarField.getTriMeshBoolean());
		setFieldValues(gradients);
	}

	//---- QUERIES

	//---- zScalar &  zVector specilization for getNeighbour_Contained
	
	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getNeighbour_Contained(zPoint &pos, vector<int> &containedNeighbour)
	{
		containedNeighbour.clear();

		zItMeshScalarField s(*fieldObj, pos);

		int index = s.getId();	

		int numRings = 1;
		//printf("\n working numRings : %i ", numRings);

		int idX, idY;
		s.getIndices(idX, idY);		

		int startIdX = -numRings;
		if (idX == 0) startIdX = 0;

		int startIdY = -numRings;
		if (idY == 0) startIdY = 0;

		int endIdX = numRings;
		if (idX == fieldObj->field.n_X) endIdX = 0;

		int endIdY = numRings;
		if (idY == fieldObj->field.n_Y) endIdY = 0;

		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				int newId_X = idX + i;
				int newId_Y = idY + j;

				int newId = (newId_X * fieldObj->field.n_Y) + (newId_Y);
			}

		}

		// case 1
		if (checkBounds_X(idX - 1) && checkBounds_Y(idY + 1))
		{
			zItMeshScalarField g1(*fieldObj, idX - 1, idY);

			zItMeshScalarField g2(*fieldObj, idX, idY);

			zItMeshScalarField g3(*fieldObj, idX, idY + 1);

			zItMeshScalarField g4(*fieldObj, idX - 1, idY + 1);

			zVector minBB_temp = g1.getPosition();
			if (!setValuesperVertex) minBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			zVector maxBB_temp = g3.getPosition();
			if (!setValuesperVertex) maxBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			bool check = coreUtils.pointInBounds(pos, minBB_temp, maxBB_temp);

			if (check)
			{
				containedNeighbour.push_back(g1.getId());
				containedNeighbour.push_back(g2.getId());
				containedNeighbour.push_back(g3.getId());
				containedNeighbour.push_back(g4.getId());
			}



		}

		// case 2
		if (containedNeighbour.size() == 0 && checkBounds_X(idX + 1) && checkBounds_Y(idY + 1))
		{

			zItMeshScalarField g1(*fieldObj, idX, idY);

			zItMeshScalarField g2(*fieldObj, idX + 1, idY);

			zItMeshScalarField g3(*fieldObj, idX + 1, idY + 1);

			zItMeshScalarField g4(*fieldObj, idX, idY + 1);

			zVector minBB_temp = g1.getPosition();
			if (!setValuesperVertex) minBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			zVector maxBB_temp = g3.getPosition();
			if (!setValuesperVertex) maxBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			bool check = coreUtils.pointInBounds(pos, minBB_temp, maxBB_temp);

			if (check)
			{
				containedNeighbour.push_back(g1.getId());
				containedNeighbour.push_back(g2.getId());
				containedNeighbour.push_back(g3.getId());
				containedNeighbour.push_back(g4.getId());
			}

		}

		// case 3
		if (containedNeighbour.size() == 0 && checkBounds_X(idX + 1) && checkBounds_Y(idY - 1))
		{
			zItMeshScalarField g1(*fieldObj, idX, idY - 1);

			zItMeshScalarField g2(*fieldObj, idX + 1, idY - 1);

			zItMeshScalarField g3(*fieldObj, idX + 1, idY);

			zItMeshScalarField g4(*fieldObj, idX, idY);

			zVector minBB_temp = g1.getPosition();
			if (!setValuesperVertex) minBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			zVector maxBB_temp = g3.getPosition();
			if (!setValuesperVertex) maxBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			bool check = coreUtils.pointInBounds(pos, minBB_temp, maxBB_temp);

			if (check)
			{
				containedNeighbour.push_back(g1.getId());
				containedNeighbour.push_back(g2.getId());
				containedNeighbour.push_back(g3.getId());
				containedNeighbour.push_back(g4.getId());
			}
		}


		// case 4
		if (containedNeighbour.size() == 0 && checkBounds_X(idX - 1) && checkBounds_Y(idY - 1))
		{

			zItMeshScalarField g1(*fieldObj, idX - 1, idY - 1);

			zItMeshScalarField g2(*fieldObj, idX, idY - 1);

			zItMeshScalarField g3(*fieldObj, idX, idY);

			zItMeshScalarField g4(*fieldObj, idX - 1, idY);


			zVector minBB_temp = g1.getPosition();
			if (!setValuesperVertex) minBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			zVector maxBB_temp = g3.getPosition();
			if (!setValuesperVertex) maxBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			bool check = coreUtils.pointInBounds(pos, minBB_temp, maxBB_temp);

			if (check)
			{
				containedNeighbour.push_back(g1.getId());
				containedNeighbour.push_back(g2.getId());
				containedNeighbour.push_back(g3.getId());
				containedNeighbour.push_back(g4.getId());
			}
		}


	}
	
	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zVector>::getNeighbour_Contained(zPoint &pos, vector<int> &containedNeighbour)
	{
		containedNeighbour.clear();

		zItMeshVectorField s(*fieldObj, pos);

		int index = s.getId();

		int numRings = 1;
		//printf("\n working numRings : %i ", numRings);

		int idX, idY;
		s.getIndices(idX, idY);

		int startIdX = -numRings;
		if (idX == 0) startIdX = 0;

		int startIdY = -numRings;
		if (idY == 0) startIdY = 0;

		int endIdX = numRings;
		if (idX == fieldObj->field.n_X) endIdX = 0;

		int endIdY = numRings;
		if (idY == fieldObj->field.n_Y) endIdY = 0;

		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				int newId_X = idX + i;
				int newId_Y = idY + j;

				int newId = (newId_X * fieldObj->field.n_Y) + (newId_Y);
			}

		}

		// case 1
		if (checkBounds_X(idX - 1) && checkBounds_Y(idY + 1))
		{
			zItMeshVectorField g1(*fieldObj, idX - 1, idY);

			zItMeshVectorField g2(*fieldObj, idX, idY);

			zItMeshVectorField g3(*fieldObj, idX, idY + 1);

			zItMeshVectorField g4(*fieldObj, idX -1, idY + 1);

			zVector minBB_temp = g1.getPosition();
			if (!setValuesperVertex) minBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			zVector maxBB_temp = g3.getPosition();
			if (!setValuesperVertex) maxBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			bool check = coreUtils.pointInBounds(pos, minBB_temp, maxBB_temp);

			if (check)
			{
				containedNeighbour.push_back(g1.getId());
				containedNeighbour.push_back(g2.getId());
				containedNeighbour.push_back(g3.getId());
				containedNeighbour.push_back(g4.getId());
			}



		}

		// case 2
		if (containedNeighbour.size() == 0 && checkBounds_X(idX + 1) && checkBounds_Y(idY + 1))
		{

			zItMeshVectorField g1(*fieldObj, idX, idY);

			zItMeshVectorField g2(*fieldObj, idX + 1, idY);

			zItMeshVectorField g3(*fieldObj, idX + 1, idY + 1);

			zItMeshVectorField g4(*fieldObj, idX, idY + 1);		

			zVector minBB_temp = g1.getPosition();
			if (!setValuesperVertex) minBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			zVector maxBB_temp = g3.getPosition();
			if (!setValuesperVertex) maxBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			bool check = coreUtils.pointInBounds(pos, minBB_temp, maxBB_temp);

			if (check)
			{
				containedNeighbour.push_back(g1.getId());
				containedNeighbour.push_back(g2.getId());
				containedNeighbour.push_back(g3.getId());
				containedNeighbour.push_back(g4.getId());
			}

		}

		// case 3
		if (containedNeighbour.size() == 0 && checkBounds_X(idX + 1) && checkBounds_Y(idY - 1))
		{
			zItMeshVectorField g1(*fieldObj, idX, idY - 1);

			zItMeshVectorField g2(*fieldObj, idX + 1, idY - 1);

			zItMeshVectorField g3(*fieldObj, idX + 1, idY);

			zItMeshVectorField g4(*fieldObj, idX, idY );		

			zVector minBB_temp = g1.getPosition();
			if (!setValuesperVertex) minBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			zVector maxBB_temp = g3.getPosition();
			if (!setValuesperVertex) maxBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			bool check = coreUtils.pointInBounds(pos, minBB_temp, maxBB_temp);

			if (check)
			{
				containedNeighbour.push_back(g1.getId());
				containedNeighbour.push_back(g2.getId());
				containedNeighbour.push_back(g3.getId());
				containedNeighbour.push_back(g4.getId());
			}
		}


		// case 4
		if (containedNeighbour.size() == 0 && checkBounds_X(idX - 1) && checkBounds_Y(idY - 1))
		{

			zItMeshVectorField g1(*fieldObj, idX -1, idY - 1);

			zItMeshVectorField g2(*fieldObj, idX , idY - 1);

			zItMeshVectorField g3(*fieldObj, idX, idY);

			zItMeshVectorField g4(*fieldObj, idX - 1, idY);
		

			zVector minBB_temp = g1.getPosition();
			if (!setValuesperVertex) minBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			zVector maxBB_temp = g3.getPosition();
			if (!setValuesperVertex) maxBB_temp -= zVector(fieldObj->field.unit_X * 0.5, fieldObj->field.unit_Y * 0.5, 0);

			bool check = coreUtils.pointInBounds(pos, minBB_temp, maxBB_temp);

			if (check)
			{
				containedNeighbour.push_back(g1.getId());
				containedNeighbour.push_back(g2.getId());
				containedNeighbour.push_back(g3.getId());
				containedNeighbour.push_back(g4.getId());
			}
		}


	}

	//---- zScalar &  zVector specilization for getNeighbourPosition_Contained
	
	template<typename T>
	ZSPACE_INLINE zObjectMesh* zFnMeshFieldBase<T>::getRawMesh()
	{
		return this->fieldObj;
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getNeighbourPosition_Contained(zPoint &pos, zPointArray &containedNeighbour)
	{
		containedNeighbour.clear();

		vector<int> cNeighbourIndex;
		getNeighbour_Contained(pos, cNeighbourIndex);

		for (int i = 0; i < cNeighbourIndex.size(); i++)
		{
			zItMeshScalarField s(*fieldObj, cNeighbourIndex[i]);

			containedNeighbour.push_back(s.getPosition());
		}

	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zVector>::getNeighbourPosition_Contained(zPoint &pos, zPointArray &containedNeighbour)
	{
		containedNeighbour.clear();

		vector<int> cNeighbourIndex;
		getNeighbour_Contained(pos, cNeighbourIndex);

		for (int i = 0; i < cNeighbourIndex.size(); i++)
		{
			zItMeshVectorField s(*fieldObj, cNeighbourIndex[i]);

			containedNeighbour.push_back(s.getPosition());
		}

	}
	
	//---- GET METHODS
	
	template<typename T>
	ZSPACE_INLINE int zFnMeshFieldBase<T>::numFieldValues()
	{
		return fieldObj->field.fieldValues.size();
	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getResolution(int &_n_X, int &_n_Y)
	{
		_n_X = fieldObj->field.n_X;
		_n_Y = fieldObj->field.n_Y;
	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getUnitDistances(double &_unit_X, double &_unit_Y)
	{
		_unit_X = fieldObj->field.unit_X;
		_unit_Y = fieldObj->field.unit_Y;

	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getBoundingBox(zPoint &_minBB, zPoint &_maxBB)
	{
		_minBB = fieldObj->field.minBB;
		_maxBB = fieldObj->field.maxBB;
	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getPositions(zPointArray &positions)
	{
		if (setValuesperVertex) 	fnMesh.getVertexPositions(positions);
		else fnMesh.getCenters(zFaceData, positions);
	}

	//---- zScalar and zVector specilization for getFieldValue

	template<>
	ZSPACE_INLINE bool zFnMeshFieldBase<zScalar>::getFieldValue(zPoint &samplePos, zFieldValueType type, zScalar& fieldValue)
	{

		bool out = false;

		zItMeshScalarField s(*fieldObj, samplePos);

		int index = s.getId();

		if (type == zFieldIndex)
		{
			zScalar fVal;

			fVal = fieldObj->field.fieldValues[index];

			fieldValue = fVal;
		}

		else if (type == zFieldNeighbourWeighted)
		{
			zScalar fVal = 0;

			zItMeshScalarField s(*fieldObj, index);

			vector<zItMeshScalarField> ringNeighbours;
			s.getNeighbour_Ring( 1, ringNeighbours);

			zPointArray positions;
			for (int i = 0; i < ringNeighbours.size(); i++)
			{
				positions.push_back(ringNeighbours[i].getPosition());
			}

			vector<double> weights;
			coreUtils.getDistanceWeights(samplePos, positions, 2.0, weights);

			double w = 0;
			for (int i = 0; i < ringNeighbours.size(); i++)
			{
				zScalar val = 	ringNeighbours[i].getValue();
				fVal += val * weights[i];
				w += weights[i];
			}

			fVal = (fVal == 0.0) ? 0.0 : fVal / w;

			fieldValue = fVal;
		}

		else if (type == zFieldAdjacentWeighted)
		{
			zScalar fVal = 0;

			zItMeshScalarField s(*fieldObj, index);

			vector<zItMeshScalarField> adjNeighbours;
			s.getNeighbour_Adjacents(adjNeighbours);

			zPointArray positions;
			for (int i = 0; i < adjNeighbours.size(); i++)
			{
				positions.push_back(adjNeighbours[i].getPosition());
			}

			vector<double> weights;
			coreUtils.getDistanceWeights(samplePos, positions, 2.0, weights);

			double w = 0;
			for (int i = 0; i < adjNeighbours.size(); i++)
			{
				zScalar val =  adjNeighbours[i].getValue();
				fVal += val * weights[i];

				w += weights[i];
			}

			fVal = (fVal == 0.0) ? 0.0 : fVal / w;

			fieldValue = fVal;
		}

		else if (type == zFieldContainedWeighted)
		{		
			zScalar fVal = 0;

			vector<int> containedNeighbours;
			getNeighbour_Contained(samplePos, containedNeighbours);

			zPointArray positions;
			for (int i = 0; i < containedNeighbours.size(); i++)
			{
				zItMeshScalarField s(*fieldObj, containedNeighbours[i]);

				positions.push_back(s.getPosition());
			}

			vector<double> weights;
			coreUtils.getDistanceWeights(samplePos, positions, 2.0, weights);

			double w = 0.0;
			for (int i = 0; i < containedNeighbours.size(); i++)
			{		
				zItMeshScalarField s(*fieldObj, containedNeighbours[i]);
				zScalar val = s.getValue();
			
				fVal += (val * weights[i]);

				w += weights[i];
			}

			fVal = (fVal == 0.0) ? 0.0 : fVal / w;

			fieldValue = fVal;

		}

		else throw std::invalid_argument(" error: invalid zFieldValueType.");

		return true;
	}

	template<>
	ZSPACE_INLINE bool zFnMeshFieldBase<zVector>::getFieldValue(zPoint &samplePos, zFieldValueType type, zVector& fieldValue)
	{

		bool out = false;

		zItMeshVectorField s(*fieldObj, samplePos);

		int index = s.getId();

		if (type == zFieldIndex)
		{
			zVector fVal;

			fVal = fieldObj->field.fieldValues[index];

			fieldValue = fVal;
		}

		else if (type == zFieldNeighbourWeighted)
		{
			zVector fVal;

			zItMeshVectorField s(*fieldObj, index);

			vector<zItMeshVectorField> ringNeighbours;
			s.getNeighbour_Ring(1, ringNeighbours);

			zPointArray positions;
			for (int i = 0; i < ringNeighbours.size(); i++)
			{
				positions.push_back(ringNeighbours[i].getPosition());
			}

			vector<double> weights;
			coreUtils.getDistanceWeights(samplePos, positions, 2.0, weights);

			double w = 0;
			for (int i = 0; i < ringNeighbours.size(); i++)
			{
				zVector val = ringNeighbours[i].getValue();
				fVal += (val * weights[i]);
				w += weights[i];
			}

			fVal /= w;

			fieldValue = fVal;
		}

		else if (type == zFieldAdjacentWeighted)
		{
			zVector fVal;

			zItMeshVectorField s(*fieldObj, index);

			vector<zItMeshVectorField> adjNeighbours;
			s.getNeighbour_Adjacents(adjNeighbours);

			zPointArray positions;
			for (int i = 0; i < adjNeighbours.size(); i++)
			{
				positions.push_back(adjNeighbours[i].getPosition());
			}

			vector<double> weights;
			coreUtils.getDistanceWeights(samplePos, positions, 2.0, weights);

			double w = 0;
			for (int i = 0; i < adjNeighbours.size(); i++)
			{
				zVector val = adjNeighbours[i].getValue();
				fVal += val * weights[i];

				w += weights[i];
			}

			fVal /= w;

			fieldValue = fVal;
		}

		else if (type == zFieldContainedWeighted)
		{

			zVector fVal;

			vector<int> containedNeighbours;
			getNeighbour_Contained(samplePos, containedNeighbours);

			zPointArray positions;
			for (int i = 0; i < containedNeighbours.size(); i++)
			{
				zItMeshVectorField s(*fieldObj, containedNeighbours[i]);

				positions.push_back(s.getPosition());
			}

			vector<double> weights;
			coreUtils.getDistanceWeights(samplePos, positions, 2.0, weights);

			double w = 0.0;
			for (int i = 0; i < containedNeighbours.size(); i++)
			{				

				zItMeshVectorField s(*fieldObj, containedNeighbours[i]);
				zVector val = s.getValue();

				fVal += (val * weights[i]);

				w += weights[i];
			}

			fVal /= w;

			fieldValue = fVal;

		}

		else throw std::invalid_argument(" error: invalid zFieldValueType.");

		return true;
	}


	template<>
	ZSPACE_INLINE bool zFnMeshFieldBase<zScalar>::getScalarValue(zScalarArray& scalars, zPoint& samplePos, zFieldValueType type, zScalar& fieldValue)
	{
		if (scalars.size() != numFieldValues())
		{
			 throw std::invalid_argument(" error: scalars and field value size dont match.");
			return false;
		}


		bool out = false;

		zItMeshScalarField s(*fieldObj, samplePos);

		int index = s.getId();

		if (type == zFieldIndex)
		{
			
			fieldValue = scalars[index];
		}

		else if (type == zFieldNeighbourWeighted)
		{
			zScalar fVal = 0;

			zItMeshScalarField s(*fieldObj, index);

			vector<zItMeshScalarField> ringNeighbours;
			s.getNeighbour_Ring(1, ringNeighbours);

			zPointArray positions;
			for (int i = 0; i < ringNeighbours.size(); i++)
			{
				positions.push_back(ringNeighbours[i].getPosition());
			}

			vector<double> weights;
			coreUtils.getDistanceWeights(samplePos, positions, 2.0, weights);

			double w = 0;
			for (int i = 0; i < ringNeighbours.size(); i++)
			{
				zScalar val = scalars[ringNeighbours[i].getId()];
				fVal += val * weights[i];
				w += weights[i];
			}

			fVal /= w;

			fieldValue = fVal;
		}

		else if (type == zFieldAdjacentWeighted)
		{
			zScalar fVal = 0;

			zItMeshScalarField s(*fieldObj, index);

			vector<zItMeshScalarField> adjNeighbours;
			s.getNeighbour_Adjacents(adjNeighbours);

			zPointArray positions;
			for (int i = 0; i < adjNeighbours.size(); i++)
			{
				positions.push_back(adjNeighbours[i].getPosition());
			}

			vector<double> weights;
			coreUtils.getDistanceWeights(samplePos, positions, 2.0, weights);

			double w = 0;
			for (int i = 0; i < adjNeighbours.size(); i++)
			{
				zScalar val = scalars[adjNeighbours[i].getId()];
				fVal += val * weights[i];

				w += weights[i];
			}

			fVal /= w;

			fieldValue = fVal;
		}

		else if (type == zFieldContainedWeighted)
		{
			zScalar fVal = 0;

			vector<int> containedNeighbours;
			getNeighbour_Contained(samplePos, containedNeighbours);

			zPointArray positions;
			for (int i = 0; i < containedNeighbours.size(); i++)
			{
				zItMeshScalarField s(*fieldObj, containedNeighbours[i]);

				positions.push_back(s.getPosition());
			}

			vector<double> weights;
			coreUtils.getDistanceWeights(samplePos, positions, 2.0, weights);

			double w = 0.0;
			for (int i = 0; i < containedNeighbours.size(); i++)
			{
				zItMeshScalarField s(*fieldObj, containedNeighbours[i]);
				zScalar val = scalars[s.getId()];

				fVal += (val * weights[i]);

				w += weights[i];
			}

			fVal /= w;

			fieldValue = fVal;

		}

		else throw std::invalid_argument(" error: invalid zFieldValueType.");

		return true;
	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getFieldValues(vector<T>& fieldValues)
	{
		fieldValues = fieldObj->field.fieldValues;
	}

	template<typename T>
	ZSPACE_INLINE T* zFnMeshFieldBase<T>::getRawFieldValues()
	{
		if (numFieldValues() == 0) throw std::invalid_argument(" error: null pointer.");

		return &fieldObj->field.fieldValues[0];
	}

	//---- zScalar specilization for getFieldValue
	template<>
	ZSPACE_INLINE zVector zFnMeshFieldBase<zScalar>::getGradient(zItMeshScalarField &s, float epsilon )
	{
		
		bool out = true;
		
		zVector samplePos = s.getPosition();

		int id_X, id_Y;
		s.getIndices(id_X, id_Y);

		if (id_X == 0 || id_Y == 0 || id_X == fieldObj->field.n_X - 1 || id_Y == fieldObj->field.n_Y - 1)
		{
			return zVector();
		}

		zItMeshScalarField s1(*fieldObj, id_X + 1, id_Y);
		zVector samplePos1 = s1.getPosition();
		zScalar fieldVal1 = s1.getValue();
		
		zItMeshScalarField s2(*fieldObj, id_X, id_Y + 1);
		zVector samplePos2 = s2.getPosition();
		zScalar fieldVal2 = s2.getValue();


		zScalar fieldVal = s.getValue();

		zScalar gX = coreUtils.ofMap(samplePos.x + epsilon, samplePos.x, samplePos1.x, fieldVal, fieldVal1) - fieldVal;
		zScalar gY = coreUtils.ofMap(samplePos.y + epsilon, samplePos.y, samplePos2.y, fieldVal, fieldVal2) - fieldVal;

		zVector gradient = zVector(gX, gY, 0);
		gradient /= (2.0 * epsilon);

		return gradient;
	}

	//---- zScalar specilization for getFieldValue
	template<>
	ZSPACE_INLINE vector<zVector> zFnMeshFieldBase<zScalar>::getGradients(float epsilon)
	{
		vector<zVector> out;

		for (zItMeshScalarField s(*fieldObj); !s.end(); s++)
		{
			out.push_back(getGradient(s, epsilon));
		}

		return out;
	}

	template<typename T>
	ZSPACE_INLINE bool zFnMeshFieldBase<T>::getValuesPerVertexBoolean()
	{
		return setValuesperVertex;
	}

	template<typename T>
	ZSPACE_INLINE bool zFnMeshFieldBase<T>::getTriMeshBoolean()
	{
		return triMesh;
	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getFieldColorDomain(zDomainColor& colDomain)
	{
		colDomain = fieldColorDomain;
	}

	//---- SET METHODS

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::setFieldColorDomain(zDomainColor &colDomain)
	{
		fieldColorDomain = colDomain;
	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::setBoundingBox(zPoint &_minBB, zPoint &_maxBB)
	{
		fieldObj->field.minBB = _minBB;
		fieldObj->field.maxBB = _maxBB;
	}
	
	//---- zScalar and zVector specilization for setFieldValues

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::setFieldValues(zScalarArray& fValues, zFieldColorType type, float sdfWidth)
	{
		if (fValues.size() == numFieldValues())
		{			
			int i = 0;
			for (zItMeshScalarField s(*fieldObj); !s.end(); s++, i++)
			{
				s.setValue(fValues[i]);
			}

			updateColors(type, sdfWidth);

			//printf("\n working  update colors %i ", contourVertexValues.size());;
		}

		else throw std::invalid_argument("input fValues size not field scalars.");
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zVector>::setFieldValues(vector<zVector>& fValues, zFieldColorType type, float sdfWidth)
	{
		if (fValues.size() == numFieldValues())
		{
			int i = 0;
			for (zItMeshVectorField s(*fieldObj); !s.end(); s++, i++)
			{
				s.setValue(fValues[i]);
			}
								
		}

		else throw std::invalid_argument("input fValues size not field vectors.");
	}

	//----  2D IDW FIELD METHODS

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getFieldValuesAsVertexDistance_IDW(vector<T> &fieldValues, zObjectMesh &inMeshObj, T meshValue, double influence, double power, bool normalise)
	{
		fieldValues.clear();
		zFnMesh inFnMesh(inMeshObj);

		zVector *meshPositions = fnMesh.getRawVertexPositions();
		zVector *inPositions = inFnMesh.getRawVertexPositions();


		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			T d;
			double wSum = 0.0;
			double tempDist = 10000;

			for (int j = 0; j < inFnMesh.numVertices(); j++)
			{
				double r = meshPositions[i].distanceTo(inPositions[j]);

				if (r < tempDist)
				{
					double w = pow(r, power);
					wSum += w;
					double val = (w > 0.0) ? ((r * influence) / (w)) : 0.0;;

					d = meshValue * val;

					tempDist = r;
				}

			}

			if (wSum > 0) d /= wSum;
			else d = T();

			fieldValues.push_back(d);
		}

		if (normalise)
		{
			normliseValues(fieldValues);
		}

	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getFieldValuesAsVertexDistance_IDW(vector<T> &fieldValues, zObjectGraph &inGraphObj, T graphValue, double influence, double power, bool normalise )
	{
		fieldValues.clear();

		zFnGraph inFngraph(inGraphObj);

		zVector *meshPositions = fnMesh.getRawVertexPositions();
		zVector *inPositions = inFngraph.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			T d;
			double wSum = 0.0;
			double tempDist = 10000;

			for (int j = 0; j < inFngraph.numVertices(); j++)
			{
				double r = meshPositions[i].distanceTo(inPositions[j]);

				if (r < tempDist)
				{
					double w = pow(r, power);
					wSum += w;
					double val = (w > 0.0) ? ((r * influence) / (w)) : 0.0;;

					d = graphValue * val;

					tempDist = r;
				}

			}

			if (wSum > 0) d /= wSum;
			else d = T();

			fieldValues.push_back(d);


		}

		if (normalise)
		{
			normliseValues(fieldValues);
		}


	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getFieldValuesAsVertexDistance_IDW(vector<T> &fieldValues, zObjectPointCloud &inPointsObj, T value, double influence, double power, bool normalise)
	{

		fieldValues.clear();
		zFnPointCloud fnPoints(inPointsObj);

		zVector *meshPositions = fnMesh.getRawVertexPositions();
		zVector *inPositions = fnPoints.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			T d;
			double wSum = 0.0;
			double tempDist = 10000;

			for (int j = 0; j < fnPoints.numVertices(); j++)
			{
				double r = meshPositions[i].distanceTo(inPositions[j]);

				double w = pow(r, power);
				wSum += w;

				double val = (w > 0.0) ? ((r * influence) / (w)) : 0.0;;

				d += (value * val);
			}


			if (wSum > 0) d /= wSum;
			else d = T();

			fieldValues.push_back(d);
		}

		if (normalise)	normliseValues(fieldValues);



	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getFieldValuesAsVertexDistance_IDW(vector<T> &fieldValues, zObjectPointCloud &inPointsObj, vector<T> &values, vector<double>& influences, double power, bool normalise)
	{
		fieldValues.clear();
		zFnPointCloud fnPoints(inPointsObj);

		if (fnPoints.numVertices() != values.size()) throw std::invalid_argument(" error: size of inPositions and values dont match.");
		if (fnPoints.numVertices() != influences.size()) throw std::invalid_argument(" error: size of inPositions and influences dont match.");

		zVector *meshPositions = fnMesh.getRawVertexPositions();
		zVector *inPositions = fnPoints.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			T d;
			double wSum = 0.0;
			double tempDist = 10000;

			for (int j = 0; j < fnPoints.numVertices(); j++)
			{
				double r = meshPositions[i].distanceTo(inPositions[j]);

				double w = pow(r, power);
				wSum += w;

				double val = (w > 0.0) ? ((r * influences[j]) / (w)) : 0.0;;

				d += (values[j] * val);
			}


			if (wSum > 0) d /= wSum;
			else d = T();

			fieldValues.push_back(d);
		}

		if (normalise)	normliseValues(fieldValues);



	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getFieldValuesAsVertexDistance_IDW(vector<T> &fieldValues, zPointArray &inPositions, T value, double influence, double power, bool normalise)
	{

		fieldValues.clear();


		zVector *meshPositions = fnMesh.getRawVertexPositions();


		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			T d;
			double wSum = 0.0;
			double tempDist = 10000;

			for (int j = 0; j < inPositions.size(); j++)
			{
				double r = meshPositions[i].distanceTo(inPositions[j]);

				double w = pow(r, power);
				wSum += w;

				double val = (w > 0.0) ? ((r * influence) / (w)) : 0.0;;

				d += (value * val);
			}


			if (wSum > 0) d /= wSum;
			else d = T();

			fieldValues.push_back(d);
		}

		if (normalise)	normliseValues(fieldValues);



	}
	
	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::getFieldValuesAsVertexDistance_IDW(vector<T> &fieldValues, zPointArray &inPositions, vector<T> &values, zDoubleArray& influences, double power, bool normalise)
	{
		if (inPositions.size() != values.size()) throw std::invalid_argument(" error: size of inPositions and values dont match.");
		if (inPositions.size() != influences.size()) throw std::invalid_argument(" error: size of inPositions and influences dont match.");

		zVector *meshPositions = fnMesh.getRawVertexPositions();


		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			T d;
			double wSum = 0.0;
			double tempDist = 10000;

			for (int j = 0; j < inPositions.size(); j++)
			{
				double r = meshPositions[i].distanceTo(inPositions[j]);

				double w = pow(r, power);
				wSum += w;

				double val = (w > 0.0) ? ((r * influences[j]) / (w)) : 0.0;;

				d += (values[j] * val);
			}


			if (wSum > 0)	d /= wSum;			
			else d = T();

			fieldValues.push_back(d);
		}

		if (normalise)	normliseValues(fieldValues);



	}

	//----  2D SCALAR FIELD METHODS

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalarsAsVertexDistance(zScalarArray &scalars, zPointArray &inPositions, bool normalise)
	{
		scalars.clear();;

		vector<float> distVals;
		float dMin = 100000;
		float dMax = 0;;

		zVector *meshPositions = fnMesh.getRawVertexPositions();


		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			distVals.push_back(100000000);
		}

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			for (int j = 0; j < inPositions.size(); j++)
			{


				double dist = meshPositions[i].squareDistanceTo(inPositions[j]);



				if (dist < distVals[i])
				{
					distVals[i] = dist;
				}
			}
		}

		scalars = distVals;
		/*dMin = coreUtils.zMin(distVals);
		dMax = coreUtils.zMax(distVals);


		for (int j = 0; j < fnMesh.numVertices(); j++)
		{
			double val = coreUtils.ofMap(distVals[j], dMin, dMax, 0.0, 1.0);
			scalars.push_back(val);
		}*/


		if (normalise)
		{
			normliseValues(scalars);
		}


	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalarsAsVertexDistance(zScalarArray &scalars, zPointArray &inPositions, float offset, bool normalise)
	{
		scalars.clear();;

		zVector *meshPositions = fnMesh.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			double d = 0.0;
			double tempDist = 10000;

			for (int j = 0; j < inPositions.size(); j++)
			{
				double r = meshPositions[i].distanceTo(inPositions[j]);

				r = r - offset;

				if (r < tempDist)
				{
					d = r;
					//d = F_of_r(r, a, b);
					tempDist = r;
				}

			}

			scalars.push_back(d);
		}

		if (normalise)
		{
			normliseValues(scalars);
		}


	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalarsAsVertexDistance(zScalarArray &scalars, zObjectPointCloud &inPointsObj, bool normalise)
	{
		scalars.clear();;

		zFnPointCloud fnPoints(inPointsObj);

		vector<double> distVals;
		double dMin = 100000;
		double dMax = 0;;

		zVector *meshPositions = fnMesh.getRawVertexPositions();
		zVector *inPositions = fnPoints.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			distVals.push_back(10000);
		}

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			for (int j = 0; j < fnPoints.numVertices(); j++)
			{


				double dist = meshPositions[i].squareDistanceTo(inPositions[j]);

				if (dist < distVals[j])
				{
					distVals[j] = dist;
				}
			}
		}

		for (int i = 0; i < distVals.size(); i++)
		{
			dMin = coreUtils.zMin(dMin, distVals[i]);
			dMax = coreUtils.zMax(dMax, distVals[i]);
		}

		for (int j = 0; j < fnMesh.numVertices(); j++)
		{
			double val = coreUtils.ofMap(distVals[j], dMin, dMax, 0.0, 1.0);
			scalars.push_back(val);
		}


		if (normalise)
		{
			normliseValues(scalars);
		}


	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalarsAsVertexDistance(zScalarArray &scalars, zObjectPointCloud &inPointsObj, float offset,  bool normalise)
	{
		scalars.clear();;

		zFnPointCloud fnPoints(inPointsObj);

		zVector *meshPositions = fnMesh.getRawVertexPositions();
		zVector *inPositions = fnPoints.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			double d = 0.0;
			double tempDist = 10000;

			for (int j = 0; j < fnPoints.numVertices(); j++)
			{
				double r = meshPositions[i].distanceTo(inPositions[j]);

				r = r - offset;

				if (r < tempDist)
				{
					d = r;
					//d = F_of_r(r, a, b);
					tempDist = r;
				}

			}

			scalars.push_back(d);
		}

		if (normalise)
		{
			normliseValues(scalars);
		}


	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalarsAsVertexDistance(zScalarArray &scalars, zObjectMesh &inMeshObj, float offset,  bool normalise)
	{
		scalars.clear();

		zFnMesh inFnMesh(inMeshObj);

		zVector *meshPositions = fnMesh.getRawVertexPositions();
		zVector *inPositions = inFnMesh.getRawVertexPositions();


		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			double d = 0.0;
			double tempDist = 10000;

			for (int j = 0; j < inFnMesh.numVertices(); j++)
			{
				double r = meshPositions[i].distanceTo(inPositions[j]);

				r = r - offset;

				if (r < tempDist)
				{
					d = r;
					//d = F_of_r(r, a, b);
					tempDist = r;
				}

			}

			scalars.push_back(d);
		}

		if (normalise)
		{
			normliseValues(scalars);
		}


	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalarsAsVertexDistance(zScalarArray &scalars, zObjectGraph &inGraphObj, float offset, bool normalise)
	{
		scalars.clear();
		zFnGraph inFnGraph(inGraphObj);

		zVector *meshPositions = fnMesh.getRawVertexPositions();
		zVector *inPositions = inFnGraph.getRawVertexPositions();

		// update values from meta balls

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			double d = 0.0;
			double tempDist = 10000;

			for (int j = 0; j < inFnGraph.numVertices(); j++)
			{
				double r = meshPositions[i].distanceTo(inPositions[j]);

				r = r - offset;

				if (r < tempDist)
				{
					d = r;
					//d = F_of_r(r, a, b);
					tempDist = r;
				}

			}

			scalars.push_back(d);
		}

		if (normalise)
		{
			normliseValues(scalars);
		}

	}
	
	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalarsAsEdgeDistance(zScalarArray &scalars, zObjectMesh &inMeshObj,float offset, bool normalise)
	{
		scalars.clear();
		zFnMesh inFnMesh(inMeshObj);

		zVector *meshPositions = fnMesh.getRawVertexPositions();
		zVector *inPositions = inFnMesh.getRawVertexPositions();

		// update values from edge distance
		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			double d = 0.0;
			double tempDist = 10000;

			for (zItMeshEdge e(inMeshObj); !e.end(); e++)
			{

				int e0 = e.getHalfEdge(0).getVertex().getId();
				int e1 = e.getHalfEdge(0).getStartVertex().getId();

				zVector closestPt;

				float r = coreUtils.minDist_Edge_Point(meshPositions[i], inPositions[e0], inPositions[e1], closestPt);

				r = r - offset;

				if (r < tempDist)
				{
					d = r;
					//d = F_of_r(r, a, b);
					tempDist = r;
				}
			}

			scalars.push_back(d);

		}

		if (normalise)
		{
			normliseValues(scalars);
		}


	}
	

	template<>
	ZSPACE_INLINE 	void zFnMeshFieldBase<zScalar>::getScalarsAsEdgeDistance(zScalarArray &scalars, zObjectGraph &inGraphObj, float offset, bool normalise)
	{
		scalars.clear();
		zFnGraph inFnGraph(inGraphObj);

		zVector *meshPositions = fnMesh.getRawVertexPositions();
		zVector *inPositions = inFnGraph.getRawVertexPositions();
				
		// update values from edge distance
		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			double d = 0.0;
			double tempDist = 10000;

			for (zItGraphEdge e(inGraphObj); !e.end(); e++)
			{

				int e0 = e.getHalfEdge(0).getVertex().getId();
				int e1 = e.getHalfEdge(0).getStartVertex().getId();

				if (e.getLength() < EPS) continue;

				zVector closestPt;

				double r = coreUtils.minDist_Edge_Point(meshPositions[i], inPositions[e0], inPositions[e1], closestPt);

				r = r - offset;

				if (r < tempDist)
				{
					d = r;					
					tempDist = r;
				}
			}

			scalars.push_back(d);

		}

		if (normalise)
		{
			normliseValues(scalars);
		}


	}


	//----  2D SD SCALAR FIELD METHODS
	
	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalars_Polygon(zScalarArray& scalars, zObjectGraph& inGraphObj, bool normalise)
	{
		scalars.clear();
		scalars.assign(fnMesh.numVertices(), 0.0);
		zFnGraph inFnGraph(inGraphObj);

		zVector* meshPositions = fnMesh.getRawVertexPositions();
		zVector* inPositions = inFnGraph.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			scalars[i] = getScalar_Polygon(inGraphObj, meshPositions[i]) ;
		}

		if (normalise)
		{
			normliseValues(scalars);
		}

	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalars_Circle(zScalarArray &scalars, zVector &cen, float r, double annularVal, bool normalise)
	{
		scalars.clear();
		scalars.assign(fnMesh.numVertices(), 0.0);

		cen.z = 0;

		zVector *meshPositions = fnMesh.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			if (annularVal == 0) scalars[i] = getScalar_Circle(cen, meshPositions[i], r);
			else scalars[i] = abs(getScalar_Circle(cen, meshPositions[i], r) - annularVal);
		}

		if (normalise) normliseValues(scalars);
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalars_Ellipse(zScalarArray& scalars, zVector& cen, float a, float b, double annularVal, bool normalise)
	{
		scalars.clear();
		scalars.assign(fnMesh.numVertices(), 0.0);

		cen.z = 0;

		zVector* meshPositions = fnMesh.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			if (annularVal == 0) scalars[i] = getScalar_Ellipse(cen, meshPositions[i], a,b);
			else scalars[i] = abs(getScalar_Ellipse(cen, meshPositions[i], a, b) - annularVal);
		}

		if (normalise) normliseValues(scalars);
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalars_Line(zScalarArray &scalars, zVector &v0, zVector &v1, double annularVal, bool normalise )
	{
		scalars.clear();
		scalars.assign(fnMesh.numVertices(), 0.0);

		zVector *meshPositions = fnMesh.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			if (annularVal == 0) scalars[i] = getScalar_Line(meshPositions[i], v0, v1);
			else scalars[i] = abs(getScalar_Line(meshPositions[i], v0, v1) - annularVal);
		}

		if (normalise) normliseValues(scalars);
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalars_Triangle(zScalarArray& scalars, zPoint& p0, zPoint& p1, zPoint& p2, double annularVal, bool normalise)
	{
		scalars.clear();
		scalars.assign(fnMesh.numVertices(), 0.0);


		p0.z = 0; p1.z = 0; p2.z = 0;

		zVector* meshPositions = fnMesh.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			if (annularVal == 0.0) scalars[i] = getScalar_Triangle(meshPositions[i], p0, p1, p2);
			else scalars[i] = abs(getScalar_Triangle(meshPositions[i], p0, p1, p2) - annularVal);
		}

		if (normalise) normliseValues(scalars);
	}


	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalars_Square(zScalarArray &scalars, zVector& cen, zVector &dimensions, float annularVal, bool normalise)
	{
		scalars.clear();
		scalars.assign(fnMesh.numVertices(), 0.0);

		cen.z = 0;

		zVector *meshPositions = fnMesh.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			zVector p = meshPositions[i];
			if (annularVal == 0) scalars[i] = getScalar_Square(p, cen, dimensions);
			else scalars[i] = abs(getScalar_Square(p, cen, dimensions) - annularVal);
		}

		if (normalise) normliseValues(scalars);
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getScalars_Trapezoid(zScalarArray &scalars, float r1, float r2, float he, float annularVal, bool normalise)
	{
		scalars.clear();
		scalars.assign(fnMesh.numVertices(), 0.0);

		zVector *meshPositions = fnMesh.getRawVertexPositions();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			zVector p = meshPositions[i];
			if (annularVal == 0) scalars[i] = getScalar_Trapezoid(p, r1, r2, he);
			else scalars[i] = abs(getScalar_Trapezoid(p, r1, r2, he) - annularVal);
		}

		if (normalise) normliseValues(scalars);
	}

	//--- COMPUTE METHODS 
	
	template<typename T>
	ZSPACE_INLINE bool zFnMeshFieldBase<T>::checkPositionBounds(zPoint &pos, int &index)
	{
		bool out = true;

		int index_X = floor((pos.x - fieldObj->field.minBB.x) / fieldObj->field.unit_X);
		int index_Y = floor((pos.y - fieldObj->field.minBB.y) / fieldObj->field.unit_Y);

		if (index_X > (fieldObj->field.n_X - 1) || index_X <  0 || index_Y >(fieldObj->field.n_Y - 1) || index_Y < 0) out = false;

		int id = index_X * fieldObj->field.n_X + index_Y;

		index = id;

		return out;
	}

	template<typename T>
	ZSPACE_INLINE bool zFnMeshFieldBase<T>::checkBounds_X(int index_X)
	{
		return (index_X < fieldObj->field.n_X && index_X >= 0);
	}

	template<typename T>
	ZSPACE_INLINE bool zFnMeshFieldBase<T>::checkBounds_Y(int index_Y)
	{
		return (index_Y < fieldObj->field.n_Y && index_Y >= 0);
	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::computeDomain(vector<T> &values, zDomain <T> &domain)
	{
		domain.min = coreUtils.zMin(values);
		domain.max = coreUtils.zMax(values);
	}

	//---- zScalar and zVector specilization for normliseValues

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::normliseValues(zScalarArray &fieldValues)
	{
		zDomainFloat d;
		computeDomain(fieldValues, d);
				
		//for (int i = 0; i < fieldValues.size(); i++) fieldValues[i] = d.max - fieldValues[i];
		//computeDomain(fieldValues, d);		

		zDomainFloat outNeg(-1.0, 0.0);
		zDomainFloat outPos(0.0, 1.0);

		zDomainFloat inNeg(d.min, 0.0);
		zDomainFloat inPos(0.0, d.max);

		for (int i = 0; i < fieldValues.size(); i++)
		{
			if(fieldValues[i] < 0) fieldValues[i] = coreUtils.ofMap(fieldValues[i], inNeg, outNeg);
			else fieldValues[i] = coreUtils.ofMap(fieldValues[i], inPos, outPos);
		}

		computeDomain(fieldValues, d);
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zVector>::normliseValues(vector<zVector> &fieldValues)
	{
		for (int i = 0; i < fieldValues.size(); i++) fieldValues[i].normalize();
	}

	//---- zScalar specilization for smoothField

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::smoothField(zScalarArray& scalars, int numSmooth, double diffuseDamp, zDiffusionType type)
	{
		for (int k = 0; k < numSmooth; k++)
		{
			zScalarArray tempValues;

			for( zItMeshScalarField s(*fieldObj); !s.end(); s++)			
			{
				float lapA = 0;

				vector<zItMeshScalarField> ringNeigbours;
				s.getNeighbour_Ring( 1, ringNeigbours);

				for (int j = 0; j < ringNeigbours.size(); j++)
				{
					int id = ringNeigbours[j].getId();
					zScalar val = scalars[id]/*ringNeigbours[j].getValue()*/;
					
					if (type == zLaplacian)
					{
						if (id != s.getId()) lapA += (val * 1);
						else lapA += (val * -8);
					}
					else if (type == zAverage)
					{
						lapA += (val * 1);
					}
				}



				if (type == zLaplacian)
				{
					float val1 = scalars[s.getId()]/*s.getValue()*/;
					
					float newA = val1 + (lapA * diffuseDamp);
					tempValues.push_back(newA);
				}
				else if (type == zAverage)
				{
					if (lapA != 0) lapA /= (ringNeigbours.size());

					tempValues.push_back(lapA);
				}

			}

			//setFieldValues(tempValues);

			scalars = tempValues;

		}

		//updateColors();
	}

	//---- zScalar & zVector specilization for computePositionsInFieldIndex

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::computePositionsInFieldIndex(zPointArray &positions, vector<zPointArray> &fieldIndexPositions)
	{
		for (int i = 0; i < numFieldValues(); i++)
		{
			vector<zVector> temp;
			fieldIndexPositions.push_back(temp);
		}


		for (int i = 0; i < positions.size(); i++)
		{
			zItMeshScalarField s(*fieldObj, positions[i]);
			int fieldIndex = s.getId();			

			fieldIndexPositions[fieldIndex].push_back(positions[i]);
		}
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zVector>::computePositionsInFieldIndex(zPointArray &positions, vector<zPointArray> &fieldIndexPositions)
	{
		for (int i = 0; i < numFieldValues(); i++)
		{
			vector<zVector> temp;
			fieldIndexPositions.push_back(temp);
		}


		for (int i = 0; i < positions.size(); i++)
		{
			zItMeshVectorField s(*fieldObj, positions[i]);
			int fieldIndex = s.getId();

			fieldIndexPositions[fieldIndex].push_back(positions[i]);
		}
	}

	//---- zScalar & zVector specilization for computePositionIndicesInFieldIndex

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::computePositionIndicesInFieldIndex(zPointArray &positions, vector<zIntArray> &fieldIndexPositionIndicies)
	{
		for (int i = 0; i < numFieldValues(); i++)
		{
			vector<int> temp;
			fieldIndexPositionIndicies.push_back(temp);
		}


		for (int i = 0; i < positions.size(); i++)
		{
			zItMeshScalarField s(*fieldObj, positions[i]);
			int fieldIndex = s.getId();

			fieldIndexPositionIndicies[fieldIndex].push_back(i);
		}
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zVector>::computePositionIndicesInFieldIndex(zPointArray &positions, vector<zIntArray> &fieldIndexPositionIndicies)
	{
		for (int i = 0; i < numFieldValues(); i++)
		{
			vector<int> temp;
			fieldIndexPositionIndicies.push_back(temp);
		}


		for (int i = 0; i < positions.size(); i++)
		{
			zItMeshVectorField s(*fieldObj, positions[i]);
			int fieldIndex = s.getId();

			fieldIndexPositionIndicies[fieldIndex].push_back(i);
		}
	}


	template<typename T>
	ZSPACE_INLINE double zFnMeshFieldBase<T>::F_of_r(double &r, double &a, double &b)
	{
		if (0 <= r && r <= b / 3.0)return (a * (1.0 - (3.0 * r * r) / (b*b)));
		if (b / 3.0 <= r && r <= b) return (3 * a / 2 * pow(1.0 - (r / b), 2.0));
		if (b <= r) return 0;


	}


	//---- SCALAR BLEND METHODS

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::blend_linear(int currentFrame, int totalFrames, zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result)
	{
		float weight = (float) currentFrame / (float) totalFrames;

		if (currentFrame <= 0) weight = 0;
		if (currentFrame >= totalFrames) weight = 1;

		fieldValues_Result.clear();
		fieldValues_Result.assign(fieldValues_A.size(), -100000);

		for (int i = 0; i < fieldValues_A.size(); i++)
		{
			fieldValues_Result[i] = (1 - weight) * fieldValues_A[i] + (weight) * fieldValues_B[i];
		}

	}

	//---- SCALAR BOOLEAN METHODS

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::boolean_union(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		vector<float> out;

		for (int i = 0; i < fieldValues_A.size(); i++)
		{
			out.push_back(coreUtils.zMin(fieldValues_A[i], fieldValues_B[i]));
		}

		if (normalise) normliseValues(out);

		fieldValues_Result = out;
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::boolean_subtract(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		vector<float> out;

		for (int i = 0; i < fieldValues_A.size(); i++)
		{
			out.push_back(coreUtils.zMax(fieldValues_A[i], -1 * fieldValues_B[i]));
		}

		if (normalise) normliseValues(out);

		fieldValues_Result = out;
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::boolean_intersect(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		vector<float> out;

		for (int i = 0; i < fieldValues_A.size(); i++)
		{
			out.push_back(coreUtils.zMax(fieldValues_A[i], fieldValues_B[i]));
		}

		if (normalise) normliseValues(out);

		fieldValues_Result = out;
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::boolean_difference(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		vector<float> AUnionB;
		boolean_union(fieldValues_A, fieldValues_B, AUnionB, normalise);

		vector<float> AIntersectB;
		boolean_intersect(fieldValues_B, fieldValues_A, AIntersectB, normalise);

		vector<float> out;
		boolean_subtract(AUnionB, AIntersectB, out, normalise);

		if (normalise) normliseValues(out);

		fieldValues_Result = out;
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::boolean_clipwithPlane(zScalarArray& scalars, zPlane& clipPlane)
	{
		int i = 0;

		for (zItMeshVertex v(*fieldObj); !v.end(); v++, i++)
		{
			zVector O(clipPlane(3, 0), clipPlane(3, 1), clipPlane(3, 2));
			zVector Z(clipPlane(2, 0), clipPlane(2, 1), clipPlane(2, 2));

			zVector A = v.getPosition() - O;
			double minDist_Plane = A * Z;
			minDist_Plane /= Z.length();

			// printf("\n dist %1.2f ", minDist_Plane);

			if (minDist_Plane > 0)
			{
				scalars[i] = 1;
			}
		}

	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::boolean_clipwithPlane(zScalarArray& fieldValues_A, zScalarArray& fieldValues_Result, zPoint& O, zVector& Z)
	{
		int i = 0;
		
		zScalarArray temp;

		for (zItMeshVertex v(*fieldObj); !v.end(); v++, i++)
		{			
			zPoint p = v.getPosition();
			float minDist_Plane = coreUtils.minDist_Point_Plane(p, O, Z);
						
			temp.push_back(minDist_Plane);
			
		}
				
		boolean_subtract(fieldValues_A, temp, fieldValues_Result, false);				

	}

	//----  UPDATE METHODS

	//---- zScalar specilization for updateColors

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::updateColors(zFieldColorType type, float sdfWidth)
	{

		vector<float> scalars;
		getFieldValues(scalars);

		if (fnMesh.numVertices() == scalars.size() || fnMesh.numPolygons() == scalars.size())
		{

			computeDomain(scalars, contourValueDomain);

			//convert to HSV

			if (contourValueDomain.min == contourValueDomain.max)
			{
				zColor col(0.5, 0.5, 0.5);

				if (fnMesh.numVertices() == scalars.size()) fnMesh.setVertexColor(col);
				else fnMesh.setFaceColor(col);

				if (fnMesh.numPolygons() == scalars.size())
				{
					contourVertexValues.clear();

					fnMesh.computeVertexColorfromFaceColor();

					for (zItMeshVertex v(*fieldObj); !v.end(); v++)
					{
						vector<int> cFaces;
						v.getConnectedFaces(cFaces);

						double val;

						for (int j = 0; j < cFaces.size(); j++)
						{
							val += scalars[cFaces[j]];
						}

						val /= cFaces.size();

						contourVertexValues.push_back(val);
					}

					computeDomain(contourVertexValues, contourValueDomain);

				}

				if (fnMesh.numVertices() == scalars.size())
				{
					contourVertexValues = scalars;

					fnMesh.computeFaceColorfromVertexColor();
				}



				return;
			}

			else
			{

				//printf("\n min %1.2f %1.2f %1.2f ", fieldColorDomain.min.r, fieldColorDomain.min.g, fieldColorDomain.min.b);
				//printf("\n max %1.2f %1.2f %1.2f ", fieldColorDomain.max.r, fieldColorDomain.max.g, fieldColorDomain.max.b);

				fieldColorDomain.min.toHSV(); fieldColorDomain.max.toHSV();
								

				zColor* cols = fnMesh.getRawVertexColors();
				if (fnMesh.numPolygons() == scalars.size()) cols = fnMesh.getRawFaceColors();

				for (int i = 0; i < scalars.size(); i++)
				{
					// SLIME
					if (type == zFieldColorType::zFieldSlime)
					{
						if (scalars[i] < contourValueDomain.min) cols[i] = fieldColorDomain.min;
						else if (scalars[i] > contourValueDomain.max) cols[i] = fieldColorDomain.max;
						else
						{
							cols[i] = coreUtils.blendColor(scalars[i], contourValueDomain, fieldColorDomain, zHSV);
						}
					}
									


					// SDFs
					if (type == zFieldColorType::zFieldSDF)
					{
						if (scalars[i] < -sdfWidth)
						{
							//temp = coreUtils.blendColor(scalars[i], dVal, dCol, zHSV);

							cols[i] = fieldColorDomain.min;/*zColor(0, 0.550, 0.950, 1);*/
						}
						else if (scalars[i] > sdfWidth)
						{

							cols[i] = /*zColor(0.25, 0.25, 0.25, 1)*/ fieldColorDomain.max;
						}
						else cols[i] = zMAGENTA;
					}
					

					//OTHER
					if (type == zFieldColorType::zFieldRegular)
					{
						cols[i] = coreUtils.blendColor(scalars[i], contourValueDomain, fieldColorDomain, zHSV);
					}					
				}

				if (fnMesh.numPolygons() == scalars.size())
				{
					contourVertexValues.clear();

					fnMesh.computeVertexColorfromFaceColor();

					for (zItMeshVertex v(*fieldObj); !v.end(); v++)
					{
						vector<int> cFaces;
						v.getConnectedFaces(cFaces);

						double val;

						for (int j = 0; j < cFaces.size(); j++)
						{
							val += scalars[cFaces[j]];
						}

						val /= cFaces.size();

						contourVertexValues.push_back(val);
					}

					computeDomain(contourVertexValues, contourValueDomain);

				}

				if (fnMesh.numVertices() == scalars.size())
				{
					contourVertexValues = scalars;

					fnMesh.computeFaceColorfromVertexColor();
				}
			}

		}
	
		//printf("\n working  colors  %i ", contourVertexValues.size());;

	}

	//---- CONTOUR METHODS
	
	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getIsocontour(zObjectGraph &coutourGraphObj, float inThreshold, int precision, float distTolerance)
	{
		if (contourVertexValues.size() == 0) return;

		if (contourVertexValues.size() != numFieldValues())
		{
			throw std::invalid_argument(" error: invalid contour condition. Call updateColors method. ");
		}

		vector<zVector> pos;
		vector<int> edgeConnects;
		zColorArray vColors;

		fnMesh.getIsoContour(contourVertexValues, inThreshold, pos, edgeConnects, vColors, precision, distTolerance);

		zFnGraph tempFn(coutourGraphObj);
		tempFn.clear();

		tempFn.create(pos, edgeConnects, PRECISION);
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getIsolineMesh(zObjectMesh &coutourMeshObj, float inThreshold, bool invertMesh)
	{
		if (contourVertexValues.size() == 0) return;
		if (contourVertexValues.size() != numFieldValues())
		{
			throw std::invalid_argument(" error: invalid contour condition.  Call updateColors method.");
		}

		fnMesh.getIsoMesh(contourVertexValues, inThreshold, invertMesh, coutourMeshObj);
	}

	template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::getIsobandMesh(zObjectMesh &coutourMeshObj, float inThresholdLow, float inThresholdHigh)
	{
		if (contourVertexValues.size() == 0) return;

		if (contourVertexValues.size() != numFieldValues())
		{
			throw std::invalid_argument(" error: invalid contour condition.  Call updateColors method.");
		}

		fnMesh.getIsobandMesh(contourVertexValues, inThresholdLow, inThresholdHigh, coutourMeshObj);
	}

	template<typename T>
	ZSPACE_INLINE void zFnMeshFieldBase<T>::createFieldMesh()
	{
		vector<zVector>positions;
		vector<int>polyConnects;
		vector<int>polyCounts;

		zVector minBB, maxBB;
		double unit_X, unit_Y;
		int n_X, n_Y;

		getUnitDistances(unit_X, unit_Y);
		getResolution(n_X, n_Y);

		int resX = n_X;
		int resY = n_Y;

		if (!setValuesperVertex)
		{
			resX++;
			resY++;
		}

		getBoundingBox(minBB, maxBB);

		zVector unitVec = zVector(unit_X, unit_Y, 0);
		zVector startPt = minBB;;

		if (!setValuesperVertex)startPt -= (unitVec * 0.5);

		for (int i = 0; i < resX; i++)
		{
			for (int j = 0; j < resY; j++)
			{
				zVector pos;
				pos.x = startPt.x + i * unitVec.x;
				pos.y = startPt.y + j * unitVec.y;

				positions.push_back(pos);

			}
		}

		/// poly connects

		for (int i = 0; i < resX - 1; i++)
		{
			for (int j = 0; j < resY - 1; j++)
			{
				int v0 = (i * resY) + j;
				int v1 = ((i + 1) * resY) + j;

				int v2 = v1 + 1;
				int v3 = v0 + 1;

				if (triMesh)
				{
					polyConnects.push_back(v0);
					polyConnects.push_back(v1);
					polyConnects.push_back(v3);
					polyCounts.push_back(3);

					polyConnects.push_back(v1);
					polyConnects.push_back(v2);
					polyConnects.push_back(v3);
					polyCounts.push_back(3);

				}
				else
				{
					polyConnects.push_back(v0);
					polyConnects.push_back(v1);
					polyConnects.push_back(v2);
					polyConnects.push_back(v3);

					polyCounts.push_back(4);
				}

			}
		}

		fnMesh.create(positions, polyCounts, polyConnects);

		printf("\n fieldmesh: v %i e %i f %i", fnMesh.numVertices(), fnMesh.numEdges(), fnMesh.numPolygons());
	}

	//---- PROTECTED SCALAR METHODS

	
	template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Polygon(zObjectGraph& inGraphObj, zPoint& p)
	{
		zItGraphVertex v(inGraphObj, 0);
		

		zItGraphHalfEdge he = v.getHalfEdge();
		zItGraphHalfEdge start = he;
	
		zPoint v0 = he.getVertex().getPosition();

		float d = (p - v0) * (p - v0);

		float s = 1.0;

		do
		{			
			
			zPoint vj = he.getStartVertex().getPosition();
			zPoint vi = he.getVertex().getPosition();

			zVector e = vj - vi;

			zVector w = p - vi;

			zVector b = w - e * coreUtils.ofClamp<float>((w * e) / (e * e), 0.0, 1.0);

			d = coreUtils.zMin(d, (b * b));

			bool c1 = (p.y >= vi.y);
			bool c2 = (p.y < vj.y);
			bool c3 = (e.x * w.y > e.y * w.x);

			if (c1 && c2 && c3) s *= -1.0;
			if (!c1 && !c2 && !c3) s *= -1.0;

			he = he.getNext();

		} while (he != start);

				
		return s * sqrt(d);
	}

	template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Circle(zPoint &cen, zPoint &p, float r)
	{
		return ((p - cen).length() - r);
	}

	template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Ellipse(zPoint& cen, zPoint& p, float a, float b)
	{
		p.x = abs(p.x);  p.y = abs(p.y); p.z = abs(p.z);
		
		zVector ab(a, b, 0);
		
		if (p.x > p.y) 
		{ 
			zPoint temp = p;
			p.x = temp.y;
			p.y = temp.x;

			temp = ab;
			ab.x = temp.y;
			ab.y = temp.x;		
		}

		float l = ab.y * ab.y - ab.x * ab.x;
		float m = ab.x * p.x / l;      float m2 = m * m;
		float n = ab.y * p.y / l;      float n2 = n * n;
		float c = (m2 + n2 - 1.0) / 3.0; float c3 = c * c * c;
		float q = c3 + m2 * n2 * 2.0;
		float d = c3 + m2 * n2;
		float g = m + m * n2;
		float co;
		if (d < 0.0)
		{
			float h = acos(q / c3) / 3.0;
			float s = cos(h);
			float t = sin(h) * sqrt(3.0);
			float rx = sqrt(-c * (s + t + 2.0) + m2);
			float ry = sqrt(-c * (s - t + 2.0) + m2);
			co = (ry + coreUtils.zSign<float>(l) * rx + abs(g) / (rx * ry) - m) / 2.0;
		}
		else
		{
			float h = 2.0 * m * n * sqrt(d);
			float s = coreUtils.zSign<float>(q + h) * pow(abs(q + h), 1.0 / 3.0);
			float u = coreUtils.zSign<float>(q - h) * pow(abs(q - h), 1.0 / 3.0);
			float rx = -s - u - c * 4.0 + 2.0 * m2;
			float ry = (s - u) * sqrt(3.0);
			float rm = sqrt(rx * rx + ry * ry);
			co = (ry / sqrt(rm - rx) + 2.0 * g / rm - m) / 2.0;
		}

		zVector m1(co, sqrt(1.0 - co * co), 0);
		zVector  r;
		r.x = ab.x  * m1.x; 
		r.y = ab.y * m1.y; 
		r.z = ab.z * m1.z;

		return (r - (p-cen)).length() * coreUtils.zSign(p.y - r.y);

		
	}

	template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Line(zPoint &p, zPoint &v0, zPoint &v1)
	{
		zVector pa = p - v0;
		zVector ba = v1 - v0;

		float h = coreUtils.ofClamp((pa* ba) / (ba* ba), 0.0f, 1.0f);

		zVector out = pa - (ba*h);


		return (out.length());
	}

	template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Triangle(zPoint& p, zPoint& p0, zPoint& p1, zPoint& p2)
	{
		zVector e0 = p1 - p0, e1 = p2 - p1, e2 = p0 - p2;
		zVector v0 = p - p0, v1 = p - p1, v2 = p - p2;
		
		
		zVector pq0 = v0 - e0 * coreUtils.ofClamp((v0 * e0) / (e0 * e0), 0.0f, 1.0f);
		zVector pq1 = v1 - e1 * coreUtils.ofClamp((v1 * e1) / (e1 * e1), 0.0f, 1.0f);
		zVector pq2 = v2 - e2 * coreUtils.ofClamp((v2 * e2) / (e2 * e2), 0.0f, 1.0f);
		
		float s = coreUtils.zSign(e0.x * e2.y - e0.y * e2.x);
		
		
		zVector d = coreUtils.zMin(coreUtils.zMin(zVector((pq0* pq0), s * (v0.x * e0.y - v0.y * e0.x),0),
			zVector((pq1* pq1), s * (v1.x * e1.y - v1.y * e1.x),0)),
			zVector((pq2* pq2), s * (v2.x * e2.y - v2.y * e2.x),0));
		

		return -sqrt(d.x) * coreUtils.zSign(d.y);
	}

	template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Square(zPoint &p, zVector& cen, zVector &dimensions)
	{
		zPoint transP = p - cen;
		transP.x = abs(transP.x); 
		transP.y = abs(transP.y); 
		transP.z = abs(transP.z);

		zVector d = transP - dimensions;
		d.x = coreUtils.zMax(d.x, 0.0f);
		d.y = coreUtils.zMax(d.y, 0.0f);
		d.z = coreUtils.zMax(d.z, 0.0f);	
		
		float r = d.length() + coreUtils.zMin (coreUtils.zMax(d.x,d.y), 0.0f);		
	
		return(r);		
	}

	template<>
	ZSPACE_INLINE double zFnMeshFieldBase<zScalar>::getScalar_Trapezoid(zPoint &p, float &r1, float &r2, float &he)
	{
		zVector k1 = zVector(r2, he, 0);
		zVector k2 = zVector((r2 - r1), (2.0 * he), 0);

		p.x = abs(p.x);
		zVector ca = zVector(p.x - coreUtils.zMin(p.x, (p.y < 0.0) ? r1 : r2), abs(p.y) - he, 0.0);
		zVector cb = p - k1 + k2 * coreUtils.ofClamp(((k1 - p) * k2) / (k2*k2), 0.0f, 1.0f);

		double s = (cb.x < 0.0 && ca.y < 0.0) ? -1.0 : 1.0;

		double out = s * sqrt(coreUtils.zMin((ca * ca), (cb * cb)));

		return out;
	}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
	// explicit instantiation
	template class zFnMeshFieldBase<zVector>;

	template class zFnMeshFieldBase<zScalar>;

#endif
}
