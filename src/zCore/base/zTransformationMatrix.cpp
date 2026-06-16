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


#include<zspace/zCore/base/zTransformationMatrix.h>
#include <src/zCore/base/internal/zTransformationMatrixImpl.h>

#include <utility>

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zTransformationMatrix::zTransformationMatrix()
		: impl_(std::make_unique<Impl>())
	{

		impl_->Transform.setIdentity();

		impl_->P.setIdentity();

		impl_->S.setIdentity();

		impl_->R.setIdentity();

		impl_->T.setIdentity();

		impl_->pivot[0] = impl_->pivot[1] = impl_->pivot[2] = 0;
		computeP();

		impl_->rotation[0] = impl_->rotation[1] = impl_->rotation[2] = 0;
		computeRx();
		computeRy();
		computeRz();
		computeR();

		impl_->scale[0] = impl_->scale[1] = impl_->scale[2] = 1;
		computeS();

		impl_->translation[0] = impl_->translation[1] = impl_->translation[2] = 0;
		computeT();

	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zTransformationMatrix::~zTransformationMatrix() = default;

	ZSPACE_INLINE zTransformationMatrix::zTransformationMatrix(const zTransformationMatrix& other)
		: impl_(std::make_unique<Impl>(*other.impl_))
	{
	}

	ZSPACE_INLINE zTransformationMatrix::zTransformationMatrix(zTransformationMatrix&& other) noexcept = default;

	ZSPACE_INLINE zTransformationMatrix& zTransformationMatrix::operator=(const zTransformationMatrix& other)
	{
		if (this != &other) *impl_ = *other.impl_;
		return *this;
	}

	ZSPACE_INLINE zTransformationMatrix& zTransformationMatrix::operator=(zTransformationMatrix&& other) noexcept = default;

	//---- SET METHODS

	ZSPACE_INLINE void zTransformationMatrix::setTransform(zTransform &inTransform, bool decompose)
	{
		if (decompose)
		{
			// set scale
			impl_->S(0, 0) = zVector(inTransform(0, 0), inTransform(0, 1), inTransform(0, 2)).length(); 
			impl_->S(1, 1) = zVector(inTransform(1, 0), inTransform(1, 1), inTransform(1, 2)).length();
			impl_->S(2, 2) = zVector(inTransform(2, 0), inTransform(2, 1), inTransform(2, 2)).length();

			// set translation
			impl_->T(3, 0) = inTransform(3, 0); impl_->T(3, 1) = inTransform(3, 1); impl_->T(3, 2) = inTransform(3, 2);

			// set rotation 
			impl_->R(0, 0) = inTransform(0, 0); impl_->R(0, 1) = inTransform(0, 1); impl_->R(0, 2) = inTransform(0, 2);
			impl_->R(1, 0) = inTransform(1, 0); impl_->R(1, 1) = inTransform(1, 1); impl_->R(1, 2) = inTransform(1, 2);
			impl_->R(2, 0) = inTransform(2, 0); impl_->R(2, 1) = inTransform(2, 1); impl_->R(2, 2) = inTransform(2, 2);

			// set pivot
			//Eigen::Matrix3f r = inTransform.block<3, 3>(0, 0);
			//Eigen::Vector3f s;
			//s[0] = inTransform.block<3, 1>(0, 0).norm();
			//s[1] = inTransform.block<3, 1>(0, 1).norm();
			//s[2] = inTransform.block<3, 1>(0, 2).norm();

			//// Compute pivot as the point that remains stationary after rotation and scaling
			//Eigen::Vector3f p = inTransform.block<3, 1>(0, 3) - r * s.asDiagonal() * inTransform.block<3, 1>(0, 3);
			//cout << "\n p \n " << inTransform.block<3, 1>(0, 3);

			//P(3, 0) = p(0);
			//P(3, 1) = p(1);
			//P(3, 2) = p(2);

			// compute components
			decomposeR();
			decomposeT();
			decomposeS();
			decomposeP();
		}

		impl_->Transform = inTransform;

	}

	ZSPACE_INLINE void zTransformationMatrix::setRotation(zFloat4 &_rotation, bool addValues)
	{
		if (addValues)
		{
			impl_->rotation[0] += DEG_TO_RAD * _rotation[0];
			impl_->rotation[1] += DEG_TO_RAD * _rotation[1];
			impl_->rotation[2] += DEG_TO_RAD * _rotation[2];
			impl_->rotation[3] = _rotation[3];
		}
		else
		{
			impl_->rotation[0] = DEG_TO_RAD * _rotation[0];
			impl_->rotation[1] = DEG_TO_RAD * _rotation[1];
			impl_->rotation[2] = DEG_TO_RAD * _rotation[2];
			impl_->rotation[3] = _rotation[3];
		}



		computeRx();
		computeRy();
		computeRz();

		computeR();
		computeTransform();
	}

	ZSPACE_INLINE void zTransformationMatrix::setScale(zFloat4 &_scale)
	{
		impl_->scale[0] = _scale[0];
		if (impl_->scale[0] == 0)impl_->scale[0] = scaleZero;

		impl_->scale[1] = _scale[1];
		if (impl_->scale[1] == 0)impl_->scale[1] = scaleZero;

		impl_->scale[2] = _scale[2];
		if (impl_->scale[2] == 0)impl_->scale[2] = scaleZero;

		impl_->scale[3] = _scale[3];
		if (impl_->scale[3] == 0)impl_->scale[3] = _scale[3];

		computeS();
		computeTransform();
	}

	ZSPACE_INLINE void zTransformationMatrix::setTranslation(zFloat4 &_translation, bool addValues)
	{
		if (addValues)
		{
			impl_->translation[0] += _translation[0];
			impl_->translation[1] += _translation[1];
			impl_->translation[2] += _translation[2];
			impl_->translation[3] = _translation[3];
		}
		else
		{
			impl_->translation[0] = _translation[0];
			impl_->translation[1] = _translation[1];
			impl_->translation[2] = _translation[2];
			impl_->translation[3] = _translation[3];
		}

		computeT();
		computeTransform();
	}

	ZSPACE_INLINE void zTransformationMatrix::setPivot(zFloat4 &_pivot)
	{
		impl_->pivot[0] = _pivot[0];
		impl_->pivot[1] = _pivot[1];
		impl_->pivot[2] = _pivot[2];
		impl_->pivot[3] = _pivot[3];

		computeP();
		computeTransform();
	}

	//---- GET METHODS

	ZSPACE_INLINE zVector zTransformationMatrix::getX()
	{
		zVector x = zVector(impl_->Transform(0, 0), impl_->Transform(0, 1), impl_->Transform(0, 2));
		x.normalize();
		return x;
	}

	ZSPACE_INLINE zVector zTransformationMatrix::getY()
	{
		zVector y = zVector(impl_->Transform(1, 0), impl_->Transform(1, 1), impl_->Transform(1, 2));;
		y.normalize();
		return y;
	}

	ZSPACE_INLINE zVector zTransformationMatrix::getZ()
	{
		zVector z = zVector(impl_->Transform(2, 0), impl_->Transform(2, 1), impl_->Transform(2, 2));
		z.normalize();
		return z;
	}

	ZSPACE_INLINE zVector zTransformationMatrix::getO()
	{
		return zVector(impl_->Transform(3, 0), impl_->Transform(3, 1), impl_->Transform(3, 2));
	}

	ZSPACE_INLINE zVector zTransformationMatrix::getTranslation()
	{
		return impl_->translation;
	}

	ZSPACE_INLINE zVector zTransformationMatrix::getPivot()
	{
		return impl_->pivot;
	}

	ZSPACE_INLINE float* zTransformationMatrix::getRawPivot()
	{
		return &impl_->pivot[0];
	}

	ZSPACE_INLINE void zTransformationMatrix::getRotation(zFloat4 &_rotation)
	{
		_rotation[0] = RAD_TO_DEG * impl_->rotation[0];
		_rotation[1] = RAD_TO_DEG * impl_->rotation[1];
		_rotation[2] = RAD_TO_DEG * impl_->rotation[2];
		_rotation[3] = impl_->rotation[3];

	}

	ZSPACE_INLINE void zTransformationMatrix::getScale(zFloat4 &_scale)
	{
		_scale[0] = impl_->scale[0];
		if (impl_->scale[0] == scaleZero)_scale[0] = 0;

		_scale[1] = impl_->scale[1];
		if (impl_->scale[1] == scaleZero)_scale[1] = 0;

		_scale[2] = impl_->scale[2];
		if (impl_->scale[2] == scaleZero)_scale[2] = 0;

		_scale[2] = impl_->scale[2];
		if (impl_->scale[2] == scaleZero)_scale[2] = 1;
	}

	//---- AS MATRIX METHODS

	ZSPACE_INLINE zTransform zTransformationMatrix::asMatrix()
	{
		return impl_->Transform;
	}

	ZSPACE_INLINE float* zTransformationMatrix::asRawMatrix()
	{
#ifndef __CUDACC__
		return impl_->Transform.data();
#else

		return impl_->Transform.getRawMatrixValues();
#endif
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::asInverseMatrix()
	{
#ifndef __CUDACC__
		return impl_->Transform.inverse();
#else
		zTransform out;
		impl_->Transform.inverse(out);
		return out;
#endif
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::asScaleMatrix()
	{
		return impl_->S;
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::asRotationMatrix()
	{
		return impl_->R;
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::asPivotMatrix()
	{
		return impl_->P;
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::asPivotTranslationMatrix()
	{
		zTransform out;
		out.setIdentity();


		out(0, 3) = -1 * impl_->pivot[0];
		out(1, 3) = -1 * impl_->pivot[1];
		out(2, 3) = -1 * impl_->pivot[2];


		return out;
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::asInversePivotTranslationMatrix()
	{

#ifndef __CUDACC__
		return asPivotTranslationMatrix().inverse();
#else
		zTransform out;
		asPivotTranslationMatrix().inverse(out);
		return out;
#endif

	}

	ZSPACE_INLINE zTransform zTransformationMatrix::asScaleTransformMatrix()
	{
		zTransform out;
		out.setIdentity();

		out(0, 0) = impl_->scale[0];
		out(1, 1) = impl_->scale[1];
		out(2, 2) = impl_->scale[2];

		out(0, 3) = (1 - impl_->scale[0]) * impl_->pivot[0];
		out(1, 3) = (1 - impl_->scale[1]) * impl_->pivot[1];
		out(2, 3) = (1 - impl_->scale[2]) * impl_->pivot[2];


		return out;
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::asInverseScaleTransformMatrix()
	{
#ifndef __CUDACC__
		return asScaleTransformMatrix().inverse();
#else
		zTransform out;
		asScaleTransformMatrix().inverse(out);
		return out;
#endif

	}

	//---- GET MATRIX METHODS

	ZSPACE_INLINE zTransform zTransformationMatrix::getWorldMatrix()
	{

		zTransform outMatrix;
		outMatrix.setIdentity();

		zVector X = getX();
		zVector Y = getY();
		zVector Z = getZ();
		zVector O = getO();


		outMatrix(0, 0) = X.x; outMatrix(0, 1) = Y.x; outMatrix(0, 2) = Z.x;
		outMatrix(1, 0) = X.y; outMatrix(1, 1) = Y.y; outMatrix(1, 2) = Z.y;
		outMatrix(2, 0) = X.z; outMatrix(2, 1) = Y.z; outMatrix(2, 2) = Z.z;

		outMatrix(0, 3) = O.x; outMatrix(1, 3) = O.y; outMatrix(2, 3) = O.z;

		return outMatrix;
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::getLocalMatrix()
	{
		zTransform outMatrix;
		outMatrix.setIdentity();

		zVector X = getX();
		zVector Y = getY();
		zVector Z = getZ();
		zVector O = getO();

		zVector orig(0, 0, 0);
		zVector d = O - orig;

		outMatrix(0, 0) = X.x; outMatrix(0, 1) = X.y; outMatrix(0, 2) = X.z;
		outMatrix(1, 0) = Y.x; outMatrix(1, 1) = Y.y; outMatrix(1, 2) = Y.z;
		outMatrix(2, 0) = Z.x; outMatrix(2, 1) = Z.y; outMatrix(2, 2) = Z.z;

		outMatrix(0, 3) = -(X*d); outMatrix(1, 3) = -(Y*d); outMatrix(2, 3) = -(Z*d);

		return outMatrix;
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::getToMatrix(zTransformationMatrix &to)
	{
		zTransform world = to.getWorldMatrix();
		zTransform local = this->getLocalMatrix();
		return world * local;

		//return this->asInverseMatrix() * to.asMatrix();
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::getToTransform(zTransformationMatrix& to)
	{
		return getToMatrix(to).transpose();
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::getBasisChangeMatrix(zTransformationMatrix &to)
	{
		zTransform local = to.getLocalMatrix();
		zTransform world = this->getWorldMatrix();

		return local * world;
	}

	ZSPACE_INLINE zTransform zTransformationMatrix::getTargetMatrix(zTransform &target)
	{
#ifndef __CUDACC__
		zTransform C_inverse = impl_->Transform.inverse();
#else
		zTransform C_inverse;
		impl_->Transform.inverse(C_inverse);
#endif
				

		zTransform targ_newbasis;
		targ_newbasis.setIdentity();

		targ_newbasis = C_inverse * target;

		return targ_newbasis;
	}

	//---- PROTECTED METHODS

	ZSPACE_INLINE void zTransformationMatrix::computeTransform()
	{
		impl_->Transform.setIdentity();

		zTransform PS = impl_->P * impl_->S;

#ifndef __CUDACC__
		zTransform PSInverse = PS.inverse();
#else
		zTransform PSInverse;
		impl_->Transform.inverse(PSInverse);
#endif

		impl_->Transform = PS * PSInverse * impl_->R * impl_->T;

		//Transform = T * P * S * R;

		// translationMatrix * pivotMatrix * scaleMatrix * rotationMatrix

	}

	ZSPACE_INLINE void zTransformationMatrix::computeR()
	{
		impl_->R.setIdentity();
		impl_->R = impl_->Rx * impl_->Ry *impl_->Rz;
	}

	ZSPACE_INLINE void zTransformationMatrix::computeRx()
	{
		impl_->Rx.setIdentity();

		impl_->Rx(1, 1) = cos(impl_->rotation[0]);
		impl_->Rx(1, 2) = sin(impl_->rotation[0]);

		impl_->Rx(2, 1) = -sin(impl_->rotation[0]);
		impl_->Rx(2, 2) = cos(impl_->rotation[0]);

	}

	ZSPACE_INLINE void zTransformationMatrix::computeRy()
	{
		impl_->Ry.setIdentity();

		impl_->Ry(0, 0) = cos(impl_->rotation[1]);
		impl_->Ry(0, 2) = -sin(impl_->rotation[1]);

		impl_->Ry(2, 0) = sin(impl_->rotation[1]);
		impl_->Ry(2, 2) = cos(impl_->rotation[1]);

	}

	ZSPACE_INLINE void zTransformationMatrix::computeRz()
	{
		impl_->Rz.setIdentity();

		impl_->Rz(0, 0) = cos(impl_->rotation[2]);
		impl_->Rz(0, 1) = sin(impl_->rotation[2]);

		impl_->Rz(1, 0) = -sin(impl_->rotation[2]);
		impl_->Rz(1, 1) = cos(impl_->rotation[2]);
	}

	ZSPACE_INLINE void zTransformationMatrix::computeT()
	{
		impl_->T.setIdentity();

		impl_->T(3, 0) = impl_->translation[0];
		impl_->T(3, 1) = impl_->translation[1];
		impl_->T(3, 2) = impl_->translation[2];
	}

	ZSPACE_INLINE void zTransformationMatrix::computeP()
	{
		impl_->P.setIdentity();

		impl_->P(3, 0) = impl_->pivot[0];
		impl_->P(3, 1) = impl_->pivot[1];
		impl_->P(3, 2) = impl_->pivot[2];
	}

	ZSPACE_INLINE void zTransformationMatrix::computeS()
	{
		impl_->S.setIdentity();

		impl_->S(0, 0) = impl_->scale[0];
		impl_->S(1, 1) = impl_->scale[1];
		impl_->S(2, 2) = impl_->scale[2];
	}

	ZSPACE_INLINE void zTransformationMatrix::decomposeR()
	{
		zFloat3 rot0;
		zFloat3 rot1;

		if (impl_->R(0, 2) != 1 && impl_->R(0, 2) != -1)
		{
			rot0[1] = asin(impl_->R(0, 2)) * -1;
			rot1[1] = Z_PI - rot0[1];

			rot0[0] = atan2(impl_->R(1, 2) / cos(rot0[1]), impl_->R(2, 2) / cos(rot0[1]));
			rot1[1] = atan2(impl_->R(1, 2) / cos(rot1[1]), impl_->R(2, 2) / cos(rot1[1]));

			rot0[2] = atan2(impl_->R(0, 1) / cos(rot0[1]), impl_->R(0, 0) / cos(rot0[1]));
			rot1[2] = atan2(impl_->R(0, 1) / cos(rot1[1]), impl_->R(0, 0) / cos(rot1[1]));

		}
		else
		{
			rot0[2] = rot1[2] = 0;

			if (impl_->R(0, 2) == -1)
			{
				rot0[1] = rot1[1] = Z_HALF_PI;
				rot0[0] = rot1[0] = atan2(impl_->R(1, 0), impl_->R(2, 0));
			}
			else
			{
				rot0[1] = rot1[1] = Z_HALF_PI * -1;
				rot0[0] = rot1[0] = atan2(impl_->R(1, 0)* -1, impl_->R(2, 0)* -1);
			}
		}

		impl_->rotation[0] = rot0[0];
		impl_->rotation[1] = rot0[1];
		impl_->rotation[2] = rot0[2];

		computeRx(); computeRy(); computeRz();
	}

	ZSPACE_INLINE void zTransformationMatrix::decomposeT()
	{
		impl_->translation[0] = impl_->T(3, 0);
		impl_->translation[1] = impl_->T(3, 1);
		impl_->translation[2] = impl_->T(3, 2);
	}

	ZSPACE_INLINE void zTransformationMatrix::decomposeS()
	{
		impl_->scale[0] = impl_->S(0, 0);
		impl_->scale[1] = impl_->S(1, 1);
		impl_->scale[2] = impl_->S(2, 2);
	}


	ZSPACE_INLINE void zTransformationMatrix::decomposeP()
	{
		impl_->pivot[0] = impl_->P(3, 0);
		impl_->pivot[1] = impl_->P(3, 1);
		impl_->pivot[2] = impl_->P(3, 2);
	}

}
