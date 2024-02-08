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


#include<headers/zInterop/functionsets/zFnNurbsCurve.h>

#include<headers/zInterOp/objects/zObjArc.h>
#include<headers/zInterop/functionsets/zFnArc.h>
#include<headers/zInterOp/objects/zObjPlane.h>
#include<headers/zInterop/functionsets/zFnPlane.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zFnNurbsCurve::zFnNurbsCurve()
	{
		fnType = zFnType::zNurbsCurveFn;
		nurbsCurveObj = nullptr;
	}

	ZSPACE_INLINE zFnNurbsCurve::zFnNurbsCurve(zObjNurbsCurve&_nurbsCurveObj)
	{
		fnType = zFnType::zNurbsCurveFn;

		nurbsCurveObj = &_nurbsCurveObj;

		
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zFnNurbsCurve::~zFnNurbsCurve() {}

	//---- OVERRIDE METHODS
	
	ZSPACE_INLINE zFnType zFnNurbsCurve::getType()
	{
		return zNurbsCurveFn;
	}

	ZSPACE_INLINE void zFnNurbsCurve::from(string path, zFileTpye type, bool staticGeom)
	{
		if (type == zJSON)
		{
			json j;

			bool chk = json_read(path, j);
			if (chk) from(j, staticGeom);			
		}

		else throw std::invalid_argument(" error: invalid zFileTpye type");
	}

	ZSPACE_INLINE void zFnNurbsCurve::from(json& j, bool staticGeom)
	{
		zUtilsJsonHE graphJSON;
		
		// read data to json 

		// Curve Attributes
		vector<zDoubleArray> cPoints;		
		json_readAttribute(j, "NC_controlPoints", cPoints);

		zDoubleArray cPointWeights;
		json_readAttribute(j, "NC_weights", cPointWeights);
		
		int cDegree;
		json_readAttribute(j, "NC_degree", cDegree);
		//nurbsCurveObj->degree = cDegree;

		bool cPeriodic;
		json_readAttribute(j, "NC_periodic", cPeriodic);
		//nurbsCurveObj->periodic = cPeriodic;
		
		//printf("\n vertexAttributes: %zi %zi", vertexAttributes.size(), vertexAttributes[0].size());

		zPointArray positions;
		const int cp_size = cPeriodic ? cPoints.size() - 3 : cPoints.size();

		for (int i = 0; i < cp_size; i++)
		{
			positions.push_back(zPoint(cPoints[i][0], cPoints[i][1], cPoints[i][2]));
		}

		create(positions, cDegree, cPeriodic, false);
	}

	ZSPACE_INLINE void zFnNurbsCurve::to(string path, zFileTpye type)
	{
		if (type == zJSON)
		{
			json j;
			to(j);
			bool chk = json_write(path, j);
			//toJSON(path);
		}

		else throw std::invalid_argument(" error: invalid zFileTpye type");
	}

	ZSPACE_INLINE void zFnNurbsCurve::to(json& j)
	{
		vector<zDoubleArray> cPoints;
		cPoints.assign(nurbsCurveObj->CVCount(), zDoubleArray());

		zDoubleArray cPointWeights;
		cPointWeights.assign(nurbsCurveObj->CVCount(), -1);

		zPointArray cvs;
		getControlPoints(cvs);
		for (int i = 0; i < nurbsCurveObj->CVCount(); i++)
		{
			cPoints[i].push_back(cvs[i].x);
			cPoints[i].push_back(cvs[i].y);
			cPoints[i].push_back(cvs[i].z);

			cPointWeights[i] = nurbsCurveObj->Weight(i);
		}

		int cDegree = getDegree();
		bool cPeriodic = isPeriodic();		

		zDomainDouble domain = getDomain();
		zDoubleArray cDomain = { domain.min, domain.max};

		zDoubleArray cKnots = getKnotVector();


		// Json file 
		json_writeAttribute(j, "NC_controlPoints", cPoints);
		json_writeAttribute(j, "NC_weights", cPointWeights);
		json_writeAttribute(j, "NC_degree", cDegree);
		json_writeAttribute(j, "NC_domain", cDomain);
		json_writeAttribute(j, "NC_knots", cKnots);
		json_writeAttribute(j, "NC_periodic", cPeriodic);
		
	}

#if defined ZSPACE_USD_INTEROP

	ZSPACE_INLINE void zFnNurbsCurve::from(UsdPrim& usd, bool staticGeom)
	{
	}

	ZSPACE_INLINE void zFnNurbsCurve::to(UsdPrim& usd)
	{
	}

#endif

	ZSPACE_INLINE void zFnNurbsCurve::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		nurbsCurveObj->getBounds(minBB, maxBB);
	}

	ZSPACE_INLINE void zFnNurbsCurve::clear()
	{
		zPointArray empty;
		nurbsCurveObj->setDisplayPositions(empty);
		//nurbsCurveObj = ON_NurbsCurve();

	}

	//---- CREATE METHODS

	ZSPACE_INLINE void zFnNurbsCurve::create(zPointArray& positions, int degree, bool periodic, bool interpolate, int displayNumPoints)
	{
		//add positions
		ON_3dPointArray points;

		for (int i = 0; i < positions.size(); i++)
			points.Append(ON_3dPoint(positions[i].x, positions[i].y, positions[i].z));

		//make curve
		if (!interpolate)
		{
			if (periodic)
				nurbsCurveObj->CreatePeriodicUniformNurbs(degree, degree + 1, points.Count(), points);
			else
				nurbsCurveObj->CreateClampedUniformNurbs(degree, degree + 1, points.Count(), points);
		}
		else
		{
			int knotStyle = 1;
			if (periodic)
			{
				knotStyle = 4;
				points.Append(points[0]);
			}
			
			*nurbsCurveObj = *RhinoInterpCurve(degree, points, (ON_3dVector*)nullptr, (ON_3dVector*)nullptr, knotStyle);

			//if (temp)
			//{
			//	nurbsCurveObj = <zObjNurbsCurve*>(temp);
			//	cout << "temp successed" << endl;

			//}

			//if (nurbsCurveObj)
			//	cout << "cast successed" << endl;
		}

		//curve properties
		//cout << endl << "cv_count:" << nurbsCurveObj->CVCount() << endl;

		//double len;
		//nurbsCurveObj->GetLength(&len);
		//cout << "curve length:" << len << endl;

		nurbsCurveObj->SetDomain(0.0, 1.0);
		nurbsCurveObj->setDisplayControlPoints(positions);

		// display points 
		setDisplayNumPoints(displayNumPoints);
	}

	ZSPACE_INLINE void zFnNurbsCurve::create(zObjGraph& o_graph, double sampleDist, int degree, bool periodic, bool interpolate, int displayNumPoints)
	{
		zPointArray positions;

		zFnGraph fnGraph(o_graph);

		fnGraph.getVertexPositions(positions);

		//re sample based on sample distance
		if (sampleDist != 0.0)
		{
			ON_3dPointArray points;

			for (int i = 0; i < positions.size(); i++)
				points.Append(ON_3dPoint(positions[i].x, positions[i].y, positions[i].z));

			ON_PolylineCurve polyline(points);

			positions.clear();

			ON_SimpleArray<ON_3dPoint> p;
			ON_SimpleArray<double> t;

			RhinoDivideCurve(polyline, 0, sampleDist, false, true, &p, &t);

			for (int i = 0; i < p.Count(); i++)
				positions.push_back(zPoint(p[i].x, p[i].y, p[i].z));
		}

		//create curve
		if (positions.size() == 2)
		{
			degree = 1;
			periodic = false;
		}
		if (positions.size() == 3) 
		{
			degree = 2;
		}

		create(positions, degree, periodic, interpolate, displayNumPoints);
	}

	ZSPACE_INLINE void zFnNurbsCurve::create(zObjArc& o_arc, int displayNumPoints)
	{
		o_arc.on_arc.GetNurbForm(*nurbsCurveObj);

		nurbsCurveObj->SetDomain(0.0, 1.0);
		setDisplayNumPoints(displayNumPoints);
	}

	//---- TOPOLOGY QUERY METHODS
	
	ZSPACE_INLINE int zFnNurbsCurve::numControlVertices()
	{
		return nurbsCurveObj->CVCount();
	}

	//---- COMPUTE METHODS
	ZSPACE_INLINE void zFnNurbsCurve::closestPoint(zPoint& testPoint, zPoint& closestPoint, double& t)
	{
		const ON_3dPoint p(testPoint.x, testPoint.y, testPoint.z);
		nurbsCurveObj->GetClosestPoint(p, &t);
		const ON_3dPoint p_curve = nurbsCurveObj->PointAt(t);

		closestPoint = zPoint(p_curve.x, p_curve.y, p_curve.z);
	}

	ZSPACE_INLINE void zFnNurbsCurve::closestPoint(zObjNurbsCurve& curveB, zPoint& closestPoint_A, zPoint& closestPoint_B, double& t_A, double& t_B)
	{
		ON_SimpleArray<const ON_Geometry*> testCrvs;
		const ON_Geometry* geo = &curveB;
		testCrvs.Append(geo);

		int obj_index;
		ON_COMPONENT_INDEX comp_index;
		double u_param, v_param;

		bool b =RhinoGetClosestPoint(*nurbsCurveObj, t_A, testCrvs, obj_index, comp_index, u_param, v_param);

		if (b)
		{
			t_B = u_param;
			ON_3dPoint pA, pB;
			pA = nurbsCurveObj->PointAt(t_A);
			pB = curveB.PointAt(t_B);
			closestPoint_A = zPoint(pA.x, pA.y, pA.z);
			closestPoint_B = zPoint(pB.x, pB.y, pB.z);
		}
	}

	ZSPACE_INLINE void zFnNurbsCurve::divideByCount(int numSegments, zPointArray& positions, zDoubleArray& tParams)
	{
		positions.clear();

		ON_SimpleArray<ON_3dPoint> p;
		ON_SimpleArray<double> t;

		RhinoDivideCurve(*nurbsCurveObj, numSegments, 0, false, true, &p, &t);

		for (int i = 0; i < p.Count(); i++)
		{
			positions.push_back(zPoint(p[i].x, p[i].y, p[i].z));
			tParams.push_back(t[i]);
		}
	}

	ZSPACE_INLINE void zFnNurbsCurve::divideByLength(double length, zPointArray& positions, zDoubleArray& tParams)
	{
		positions.clear();

		ON_SimpleArray<ON_3dPoint> p;
		ON_SimpleArray<double> t;

		RhinoDivideCurve(*nurbsCurveObj, 0, length, false, true, &p, &t);

		for (int i = 0; i < p.Count(); i++)
		{
			positions.push_back(zPoint(p[i].x, p[i].y, p[i].z));
			tParams.push_back(t[i]);
		}
	}

	ZSPACE_INLINE void zFnNurbsCurve::computeSubCurve(double t0, double t1, bool normalise, zObjNurbsCurve& out_SubCurve, int displayNumPoints)
	{
		ON_Interval interval(t0, t1);

		nurbsCurveObj->NurbsCurve(&out_SubCurve, 0.0, &interval);

		zPointArray positions;
		zFnNurbsCurve fn_out(out_SubCurve);

		if (normalise) fn_out.setDomain(0.0, 1.0);
		fn_out.setDegree(nurbsCurveObj->Degree(), displayNumPoints);
		fn_out.getControlPoints(positions);
		out_SubCurve.setDisplayControlPoints(positions);
	}

	ZSPACE_INLINE void zFnNurbsCurve::reverse()
	{
		nurbsCurveObj->Reverse();
	}

	//---- INTERSECT METHODS

	ZSPACE_INLINE void zFnNurbsCurve::intersect(zPlane& plane, zPointArray& intersectionPts, zDoubleArray& tParams)
	{

		intersectionPts.clear();
		tParams.clear();

		const ON_3dVector Z(plane(2, 0), plane(2, 1), plane(2, 2));
		const ON_3dPoint O(plane(3, 0), plane(3, 1), plane(3, 2));

		ON_PlaneEquation on_plane;
		on_plane.Create(O, Z);

		ON_SimpleArray<ON_X_EVENT> intersectionEvents;
		nurbsCurveObj->IntersectPlane(on_plane, intersectionEvents);

		ON_3dPoint cx_pt;
		double cx_param;

		for (int i = 0; i < intersectionEvents.Count(); i++)
		{
			ON_X_EVENT IE = intersectionEvents[i];
			if (IE.IsPointEvent())
			{
				cx_pt = IE.m_A[2];
				cx_param = IE.m_a[2];

				intersectionPts.push_back(zVector(cx_pt.x, cx_pt.y, cx_pt.z));
				tParams.push_back(cx_param);
			}
		}
	}

	ZSPACE_INLINE void zFnNurbsCurve::intersect(zObjNurbsCurve& curveB, zPointArray& intersectionPts_A, zPointArray& intersectionPts_B, zDoubleArray& tParams_A, zDoubleArray& tParams_B)
	{
		intersectionPts_A.clear();
		intersectionPts_B.clear();
		tParams_A.clear();
		tParams_B.clear();
		
		ON_SimpleArray<ON_X_EVENT> intersectionEvents;
		nurbsCurveObj->IntersectCurve(&curveB, intersectionEvents);

		ON_3dPoint cx_pt_a, cx_pt_b;
		double cx_param_a, cx_param_b;

		for (int i = 0; i < intersectionEvents.Count(); i++)
		{
			ON_X_EVENT IE = intersectionEvents[i];
			if (IE.IsPointEvent())
			{
				cx_pt_a = IE.m_A[2];
				cx_param_a = IE.m_a[2];

				cx_pt_b = IE.m_B[2];
				cx_param_b = IE.m_b[4];

				intersectionPts_A.push_back(zVector(cx_pt_a.x, cx_pt_a.y, cx_pt_a.z));
				intersectionPts_B.push_back(zVector(cx_pt_b.x, cx_pt_b.y, cx_pt_b.z));
				tParams_A.push_back(cx_param_a);
				tParams_B.push_back(cx_param_b);
			}
		}
	}

	ZSPACE_INLINE void zFnNurbsCurve::intersectSelf(zPointArray& intersectionPts, zDoubleArray& tParams)
	{
		intersectionPts.clear();
		tParams.clear();

		ON_SimpleArray<ON_X_EVENT> intersectionEvents;
		nurbsCurveObj->IntersectSelf(intersectionEvents);

		ON_3dPoint cx_pt;
		double cx_param;

		for (int i = 0; i < intersectionEvents.Count(); i++)
		{
			ON_X_EVENT IE = intersectionEvents[i];
			if (IE.IsPointEvent())
			{
				cx_pt = IE.m_A[2];
				cx_param = IE.m_a[2];

				intersectionPts.push_back(zVector(cx_pt.x, cx_pt.y, cx_pt.z));
				tParams.push_back(cx_param);
			}
		}
	}

	//--- SET METHODS 

	ZSPACE_INLINE void zFnNurbsCurve::setDegree(int _degree, int _displayNumPoints)
	{
		nurbsCurveObj->IncreaseDegree(_degree);
		setDisplayNumPoints(_displayNumPoints);
	}

	ZSPACE_INLINE void zFnNurbsCurve::setDomain(double t0, double t1)
	{
		nurbsCurveObj->SetDomain(t0, t1);
	}

	ZSPACE_INLINE void zFnNurbsCurve::setDisplayNumPoints(int _numPoints)
	{
		zPointArray displayPositions;
		getCurvePositions(_numPoints, displayPositions);
		nurbsCurveObj->setDisplayPositions(displayPositions);
	}

	ZSPACE_INLINE void zFnNurbsCurve::setControlPointWeights(zDoubleArray& _controlPointWeights)
	{
		for (int i = 0; i < _controlPointWeights.size(); i++)
		{
			nurbsCurveObj->SetWeight(i, _controlPointWeights[i]);
		}
	}

	//--- GET METHODS 

	ZSPACE_INLINE void zFnNurbsCurve::getControlPoints(zPointArray& positions)
	{
		positions.clear();
		int numCV = nurbsCurveObj->CVCount();
		positions.assign(numCV, zPoint());

		for (int i = 0; i < numCV; i++)
		{
			ON_3dPoint p;
			nurbsCurveObj->GetCV(i, p);
			positions[i] = zPoint(p.x, p.y, p.z);
		}
	}

	ZSPACE_INLINE zPoint* zFnNurbsCurve::getRawControlPoints()
	{
		zPointArray cvs;
		getControlPoints(cvs);

		return &cvs[0];
	}

	ZSPACE_INLINE void zFnNurbsCurve::getCurvePositions(int numPoints, zPointArray& positions)
	{
		positions.clear();

		if (nurbsCurveObj->Dimension() == 1)
		{
			 getControlPoints(positions);
		}
		else
		{
			zDoubleArray t;
			divideByCount(numPoints - 1, positions, t);
			
			//for (int i = 0; i < numPoints; i++)
			//{
			//	const double s = (1.0 / (numPoints - 1)) * i;
			//	double t = 0.0;
			//	nurbsCurveObj->GetNormalizedArcLengthPoint(s,&t);
			//	//const ON_3dPoint p = nurbsCurveObj->PointAt(t);
			//	const ON_3dPoint p = nurbsCurveObj->PointAt(s);
			//	cout << "s:" << s << endl;
			//	cout << "t:" << t << endl;

			//	positions.push_back(zPoint(p.x, p.y, p.z));
			//}
		}
	}

	ZSPACE_INLINE zPoint zFnNurbsCurve::getPointAt(double t)
	{
		ON_3dPoint p = nurbsCurveObj->PointAt(t);
		return zPoint(p.x, p.y, p.z);
	}

	ZSPACE_INLINE zVector zFnNurbsCurve::getTangentAt(double t)
	{
		ON_3dVector tan = nurbsCurveObj->TangentAt(t);
		return zVector(tan.x, tan.y, tan.z);
	}

	ZSPACE_INLINE void zFnNurbsCurve::getPointTangentAt(double t, zPoint& p, zVector& tan)
	{
		p = getPointAt(t);
		tan = getTangentAt(t);
	}

	ZSPACE_INLINE zPoint* zFnNurbsCurve::getRawDisplayPositions()
	{
		zPointArray displayPositions;
		nurbsCurveObj->getDisplayPositions(displayPositions);
		return &displayPositions[0];
	}

	ZSPACE_INLINE zPoint zFnNurbsCurve::getCenter()
	{
		ON_3dPoint p = nurbsCurveObj->PointAt(0.5);
		return zPoint(p.x, p.y, p.z);
	}

	ZSPACE_INLINE double zFnNurbsCurve::getLength()
	{
		//double len = 0.0;
		//ON_NurbsCurve nCurve = nurbsCurveObj;
		//nCurve.GetLength(&len);
		//ON_NurbsCurve_GetLength(nCurve, &len);
		double len;
		nurbsCurveObj->GetLength(&len);
		return len;
	
	}

	ZSPACE_INLINE zDomainDouble zFnNurbsCurve::getDomain()
	{
		zDomainDouble out;
		nurbsCurveObj->GetDomain(&out.min, &out.max);

		return out;
	}

	ZSPACE_INLINE zDoubleArray zFnNurbsCurve::getKnotVector()
	{
		zDoubleArray out;
		out.assign(nurbsCurveObj->KnotCount(), -1);
		
		for (int i = 0; i < nurbsCurveObj->KnotCount(); i++)
		{
			out[i] = nurbsCurveObj->m_knot[i];
		}
		

		return out;
	}

	ZSPACE_INLINE int zFnNurbsCurve::getDegree()
	{
		return nurbsCurveObj->Dimension();
	}

	ZSPACE_INLINE bool zFnNurbsCurve::isPeriodic()
	{
		return nurbsCurveObj->IsPeriodic();
	}

	ZSPACE_INLINE ON_NurbsCurve* zFnNurbsCurve::getRawON_Curve()
	{
		return nurbsCurveObj;
	}


	//---- TRANSFORM METHODS OVERRIDES

	ZSPACE_INLINE void zFnNurbsCurve::setTransform(zTransform &inTransform, bool decompose, bool updatePositions)
	{
		if (updatePositions)
		{
			zTransformationMatrix to;
			to.setTransform(inTransform, decompose);

			zTransform transMat = nurbsCurveObj->transformationMatrix.getToMatrix(to);
			transformObject(transMat);

			nurbsCurveObj->transformationMatrix.setTransform(inTransform);

			// update pivot values of object transformation matrix
			zVector p = nurbsCurveObj->transformationMatrix.getPivot();
			p = p * transMat;
			setPivot(p);

		}
		else
		{
			nurbsCurveObj->transformationMatrix.setTransform(inTransform, decompose);

			zVector p = nurbsCurveObj->transformationMatrix.getO();
			setPivot(p);

		}

	}

	ZSPACE_INLINE void zFnNurbsCurve::setScale(zFloat4 &scale)
	{
		// get  inverse pivot translations
		zTransform invScalemat = nurbsCurveObj->transformationMatrix.asInverseScaleTransformMatrix();

		// set scale values of object transformation matrix
		nurbsCurveObj->transformationMatrix.setScale(scale);

		// get new scale transformation matrix
		zTransform scaleMat = nurbsCurveObj->transformationMatrix.asScaleTransformMatrix();

		// compute total transformation
		zTransform transMat = invScalemat * scaleMat;

		// transform object
		transformObject(transMat);
	}

	ZSPACE_INLINE void zFnNurbsCurve::setRotation(zFloat4 &rotation, bool appendRotations)
	{
		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = nurbsCurveObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = nurbsCurveObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = nurbsCurveObj->transformationMatrix;
		to.setRotation(rotation, appendRotations);
		zTransform toMat = nurbsCurveObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set rotation values of object transformation matrix
		nurbsCurveObj->transformationMatrix.setRotation(rotation, appendRotations);;
	}

	ZSPACE_INLINE void zFnNurbsCurve::setTranslation(zVector &translation, bool appendTranslations)
	{
		// get vector as zDouble3
		zFloat4 t;
		translation.getComponents(t);

		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = nurbsCurveObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = nurbsCurveObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = nurbsCurveObj->transformationMatrix;
		to.setTranslation(t, appendTranslations);
		zTransform toMat = nurbsCurveObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set translation values of object transformation matrix
		nurbsCurveObj->transformationMatrix.setTranslation(t, appendTranslations);;

		// update pivot values of object transformation matrix
		zVector p = nurbsCurveObj->transformationMatrix.getPivot();
		p = p * transMat;
		setPivot(p);

	}

	ZSPACE_INLINE void zFnNurbsCurve::setPivot(zVector &pivot)
	{
		// get vector as zDouble3
		zFloat4 p;
		pivot.getComponents(p);

		// set pivot values of object transformation matrix
		nurbsCurveObj->transformationMatrix.setPivot(p);
	}

	ZSPACE_INLINE void zFnNurbsCurve::getTransform(zTransform &transform)
	{
		transform = nurbsCurveObj->transformationMatrix.asMatrix();
	}

	//---- PROTECTED OVERRIDE METHODS

	ZSPACE_INLINE void zFnNurbsCurve::transformObject(zTransform &transform)
	{

		if (numControlVertices() == 0) return;

	
		zPoint* pos = getRawControlPoints();

		for (int i = 0; i < numControlVertices(); i++)
		{
			zVector newPos = pos[i] * transform;
			pos[i] = newPos;
			ON_3dPoint p(pos[i].x, pos[i].y, pos[i].z);
			nurbsCurveObj->SetCV(i, p);
		}

		// display points 
		zPointArray displayControlPoints;
		getControlPoints(displayControlPoints);
		nurbsCurveObj->setDisplayControlPoints(displayControlPoints);

		zPointArray displayPositions;
		int numPoints = nurbsCurveObj->getNumDisplayPositions();
		getCurvePositions(numPoints, displayPositions);
		nurbsCurveObj->setDisplayPositions(displayPositions);
	}


	//---- PRIVATE METHODS

	ZSPACE_INLINE bool zFnNurbsCurve::ON_NurbsCurve_GetLength(const ON_NurbsCurve& curve, double* length, double fractional_tolerance, const ON_Interval* sub_domain)
	{
		return curve.GetLength(length, fractional_tolerance, sub_domain);
	}


}