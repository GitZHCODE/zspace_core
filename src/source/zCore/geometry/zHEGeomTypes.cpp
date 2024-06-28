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


#include "zCore/geometry/zHEGeomTypes.h"

//---- ZEDGE ------------------------------------------------------------------------------

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zEdge::zEdge()
	{
		index = -1;
		he[0] = -1;
		he[1] = -1;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zEdge::~zEdge() {}

	//---- GET-SET METHODS

	ZSPACE_INLINE int zEdge::getId()
	{
		return this->index;
	}

	ZSPACE_INLINE void zEdge::setId(int _edgeId)
	{
		this->index = _edgeId;
	}

	ZSPACE_INLINE int zEdge::getHalfEdge(int _index)
	{
		return this->he[_index];
	}

	ZSPACE_INLINE void zEdge::setHalfEdge(int _he, int _index)
	{
		this->he[_index] = _he;
	}

	//---- METHODS

	ZSPACE_INLINE bool zEdge::isActive() const
	{
		return index >= 0;
	}

	ZSPACE_INLINE void zEdge::reset()
	{
		he[0] = -1;
		he[1] = -1;
		index = -1;
	}
}

//---- ZHALFEDGE ------------------------------------------------------------------------------

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zHalfEdge::zHalfEdge()
	{
		index = -1;
		v = -1;
		f = -1;
		e = -1;

		prev = -1;
		next = -1;
		sym = -1;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zHalfEdge::~zHalfEdge(){}

	//---- GET-SET METHODS

	ZSPACE_INLINE int zHalfEdge::getId()
	{
		return this->index;
	}

	ZSPACE_INLINE void zHalfEdge::setId(int _halfedgeId)
	{
		this->index = _halfedgeId;
	}

	ZSPACE_INLINE int zHalfEdge::getSym()
	{
		return this->sym;
	}

	ZSPACE_INLINE void zHalfEdge::setSym(int _sym)
	{
		this->sym = _sym;
		//_sym->sym = this->index;
	}

	ZSPACE_INLINE int zHalfEdge::getPrev()
	{
		return this->prev;
	}

	ZSPACE_INLINE void zHalfEdge::setPrev(int _prev)
	{
		this->prev = _prev;
		//if (this->getPrev()) _prev->next = this;
	}

	ZSPACE_INLINE int zHalfEdge::getNext()
	{
		return this->next;
	}

	ZSPACE_INLINE void zHalfEdge::setNext(int _next)
	{
		this->next = _next;
		//if (this->getNext()) _next->prev = this;
	}

	ZSPACE_INLINE int zHalfEdge::getVertex()
	{
		return this->v;
	}

	ZSPACE_INLINE void zHalfEdge::setVertex(int _v)
	{
		this->v = _v;
	}

	ZSPACE_INLINE int zHalfEdge::getFace()
	{
		return this->f;
	}

	ZSPACE_INLINE void zHalfEdge::setFace(int _f)
	{
		this->f = _f;
	}

	ZSPACE_INLINE int zHalfEdge::getEdge()
	{
		return this->e;
	}

	ZSPACE_INLINE void zHalfEdge::setEdge(int _e)
	{
		this->e = _e;
	}

	//---- METHODS

	ZSPACE_INLINE void zHalfEdge::reset()
	{
		//if (this->getNext()) this->getNext()->setPrev(this->getSym()->getPrev());
		//if (this->getPrev()) this->getPrev()->setNext(this->getSym()->getNext());


		this->next = -1;
		this->prev = -1;
		this->v = -1;
		this->f = -1;

		/*if (this->getSym())
		{
			this->getSym()->next = nullptr;
			this->getSym()->prev = nullptr;
			this->getSym()->v = nullptr;
			this->getSym()->f = nullptr;
		}*/

		index = -1;

	}

	ZSPACE_INLINE bool zHalfEdge::isActive() const
	{
		return index >= 0;
	}

}

//---- ZVERTEX ---------------------------------------------------------------------------------

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zVertex::zVertex()
	{
		index = -1;
		he = -1;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zVertex::~zVertex() {}

	//---- GET-SET METHODS

	ZSPACE_INLINE int zVertex::getId()
	{
		return this->index;
	}

	ZSPACE_INLINE void zVertex::setId(int _vertexId)
	{
		this->index = _vertexId;
	}

	ZSPACE_INLINE int zVertex::getHalfEdge()
	{
		return this->he;
	}

	ZSPACE_INLINE void zVertex::setHalfEdge(int _he)
	{
		this->he = _he;
	}

	//---- METHODS

	ZSPACE_INLINE void zVertex::reset()
	{
		this->he = -1;
		index = -1;
	}

	ZSPACE_INLINE bool zVertex::isActive() const
	{
		return index >= 0;
	}

}

//---- ZFACE -----------------------------------------------------------------------------------

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zFace::zFace()
	{
		index = -1;
		he = -1;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zFace::~zFace(){}

	//---- GET-SET METHODS

	ZSPACE_INLINE int zFace::getId()
	{
		return this->index;
	}

	ZSPACE_INLINE void zFace::setId(int _faceId)
	{
		this->index = _faceId;
	}

	ZSPACE_INLINE int zFace::getHalfEdge()
	{
		return this->he;
	}

	ZSPACE_INLINE void zFace::setHalfEdge(int _he)
	{
		this->he = _he;
	}

	//---- METHODS

	ZSPACE_INLINE void zFace::reset()
	{
		this->he = -1;
		index = -1;
	}

	ZSPACE_INLINE bool zFace::isActive() const
	{
		return index >= 0;
	}
}