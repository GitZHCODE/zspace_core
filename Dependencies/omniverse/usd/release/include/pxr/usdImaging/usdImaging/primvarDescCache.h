//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#ifndef PXR_USD_IMAGING_USD_IMAGING_PRIMVARDESC_CACHE_H
#define PXR_USD_IMAGING_USD_IMAGING_PRIMVARDESC_CACHE_H

/// \file usdImaging/primvarDescCache.h

#include "pxr/pxr.h"
#include "pxr/usdImaging/usdImaging/api.h"
#include "pxr/imaging/hd/sceneDelegate.h"

#include "pxr/usd/sdf/path.h"

#include "pxr/base/tf/token.h"

#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_queue.h>

PXR_NAMESPACE_OPEN_SCOPE

/// \class UsdImagingPrimvarDescCache
///
/// A cache for primvar descriptors.
///
class UsdImagingPrimvarDescCache 
{
public:
    UsdImagingPrimvarDescCache(const UsdImagingPrimvarDescCache&) = delete;
    UsdImagingPrimvarDescCache& operator=(const UsdImagingPrimvarDescCache&) 
        = delete;

    class Key 
    {
        friend class UsdImagingPrimvarDescCache;
        SdfPath _path;
        TfToken _attribute;
    public:
		Key() {}
        Key(SdfPath const& path, TfToken const& attr)
            : _path(path)
            , _attribute(attr)
        {}

        inline bool operator==(Key const& rhs) const {
            return _path == rhs._path && _attribute == rhs._attribute;
        }
        inline bool operator!=(Key const& rhs) const {
            return !(*this == rhs);
        }

        struct Hash {
            inline size_t operator()(Key const& key) const {
                size_t hash = key._path.GetHash();
                // nv begin - usdrt wouldn't compile on boost 1.76 without this change
                boost::hash_combine<size_t>(hash, key._attribute.Hash());
                // nv end
                return hash;
            }
        };

    private:
        static Key Primvars(SdfPath const& path) {
            static TfToken attr("primvars");
            return Key(path, attr);
        }
		//+NV_CHANGE FRZHANG : NV GPU Skinning
		//skinned prim keys
		static Key RestPoints(SdfPath const& path) {
			static TfToken attr("restPoints");
			return Key(path, attr);
		}
		static Key GeomBindXform(SdfPath const& path) {
			static TfToken attr("geomBindXform");
			return Key(path, attr);
		}
		static Key JointIndices(SdfPath const& path) {
			static TfToken attr("jointIndices");
			return Key(path, attr);
		}
		static Key JointWeights(SdfPath const& path) {
			static TfToken attr("jointWeights");
			return Key(path, attr);
		}
		static Key NumInfluencesPerPoint(SdfPath const& path) {
			static TfToken attr("numInfluencesPerPoint");
			return Key(path, attr);
		}
		static Key HasConstantInfluences(SdfPath const& path) {
			static TfToken attr("hasConstantInfluences");
			return Key(path, attr);
		}
        static Key SkinningMethod(SdfPath const& path) {
            static TfToken attr("skinningMethod");
            return Key(path, attr);
        }
        static Key SkinningBlendWeights(SdfPath const& path) {
            static TfToken attr("skinningBlendWeights");
            return Key(path, attr);
        }
        static Key HasConstantSkinningBlendWeights(SdfPath const& path) {
            static TfToken attr("hasConstantSkinningBlendWeights");
            return Key(path, attr);
        }
        //skeleton prim keys
		static Key PrimWorldToLocal(SdfPath const& path) {
			static TfToken attr("primWorldToLocal");
			return Key(path, attr);
		}
		static Key SkinningXforms(SdfPath const& path) {
			static TfToken attr("skinningXforms");
			return Key(path, attr);
		}
		static Key SkelLocalToWorld(SdfPath const& path) {
			static TfToken attr("skelLocalToWorld");
			return Key(path, attr);
		}
		//-NV_CHANGE FRZHANG
    };

    UsdImagingPrimvarDescCache()
        : _locked(false)
    { }

private:
    template <typename Element>
    struct _TypedCache
    {
        typedef tbb::concurrent_unordered_map<Key, Element, Key::Hash> _MapType;
        typedef typename _MapType::iterator                            _MapIt;
        typedef typename _MapType::const_iterator                      _MapConstIt;
        typedef tbb::concurrent_queue<Key>                          _QueueType;

        _MapType   _map;
    };


    /// Locates the requested \p key then populates \p value and returns true if
    /// found.
    template <typename T>
    bool _Find(Key const& key, T* value) const {
        typedef _TypedCache<T> Cache_t;

        Cache_t *cache = nullptr;

        _GetCache(&cache);
        typename Cache_t::_MapConstIt it = cache->_map.find(key);
        if (it == cache->_map.end()) {
            return false;
        }
        *value = it->second;
        return true;
    }

#if 0
    /// Locates the requested \p key then populates \p value, swap the value
    /// from the entry and queues the entry up for deletion.
    /// Returns true if found.
    /// This function is thread-safe, but Garbage collection must be called
    /// to perform the actual deletion.
    /// Note: second hit on same key will be sucessful, but return whatever
    /// value was passed into the first _Extract.
    template <typename T>
    bool _Extract(Key const& key, T* value) {
        if (!TF_VERIFY(!_locked)) {
            return false;
        }
      
        typedef _TypedCache<T> Cache_t;
        Cache_t *cache = nullptr;

        _GetCache(&cache);
        typename Cache_t::_MapIt it = cache->_map.find(key);

        if (it == cache->_map.end()) {
            return false;
        }

        // If we're going to erase the old value, swap to avoid a copy.
        //+NV_CHANGE FRZHANG
		//looks like we do need a copy instead of delete this resource. Hope no side effect.
#define PERMANENT_CACHE 0
#if PERMANENT_CACHE
		*value = it->second;
#else
		std::swap(it->second, *value);
		cache->_deferredDeleteQueue.push(key);
#endif
		//-NV_CHANGE FRZHANG
        return true;
    }

#endif

    /// Erases the given key from the value cache.
    /// Not thread safe
    template <typename T>
    void _Erase(Key const& key) {
        if (!TF_VERIFY(!_locked)) {
            return;
        }

        typedef _TypedCache<T> Cache_t;

        Cache_t *cache = nullptr;
        _GetCache(&cache);
        cache->_map.unsafe_erase(key);
    }

    /// Returns a reference to the held value for \p key. Note that the entry
    /// for \p key will created with a default-constructed instance of T if
    /// there was no pre-existing entry.
    template <typename T>
    T& _Get(Key const& key) const {
        typedef _TypedCache<T> Cache_t;

        Cache_t *cache = nullptr;
        _GetCache(&cache);

        // With concurrent_unordered_map, multi-threaded insertion is safe.
        std::pair<typename Cache_t::_MapIt, bool> res =
                                cache->_map.insert(std::make_pair(key, T()));

        return res.first->second;
    }

    // XXX:aluk
    // TODO: Resolve merge conflict
#if 0
    /// Removes items from the cache that are marked for deletion.
    /// This is not thread-safe and designed to be called after
    /// all the worker threads have been joined.
    template <typename T>
    void _GarbageCollect(_TypedCache<T> &cache) {
        typedef _TypedCache<T> Cache_t;

		Key key;
        while (cache._deferredDeleteQueue.try_pop(key))
        {
            auto itor = cache._map.find(key);
            if (itor != cache._map.end())
            {
                cache._map.unsafe_erase(itor);
            }
        }
    }

#endif

public:

    void EnableMutation() { _locked = false; }
    void DisableMutation() { _locked = true; }

    /// Clear all data associated with a specific path.
    void Clear(SdfPath const& path) {
		//+NV_CHANGE FRZHANG : NV GPU Skinning
		_Erase<VtValue>(Key::RestPoints(path));
		_Erase<GfMatrix4d>(Key::GeomBindXform(path));
		_Erase<VtValue>(Key::JointIndices(path));
		_Erase<VtValue>(Key::JointWeights(path));
		_Erase<int>(Key::NumInfluencesPerPoint(path));
		_Erase<bool>(Key::HasConstantInfluences(path));
        _Erase<TfToken>(Key::SkinningMethod(path));
        _Erase<VtValue>(Key::SkinningBlendWeights(path));
        _Erase<bool>(Key::HasConstantSkinningBlendWeights(path));
		_Erase<GfMatrix4d>(Key::PrimWorldToLocal(path));
		_Erase<VtValue>(Key::SkinningXforms(path));
		_Erase<GfMatrix4d>(Key::SkelLocalToWorld(path));
		//-NV_CHANGE FRZHANG

        _Erase<HdPrimvarDescriptorVector>(Key::Primvars(path));
    }

    HdPrimvarDescriptorVector& GetPrimvars(SdfPath const& path) const {
        return _Get<HdPrimvarDescriptorVector>(Key::Primvars(path));
    }
	//+NV_CHANGE FRZHANG : NV GPU Skinning
	VtValue& GetRestPoints(SdfPath const& path) const {
		return _Get<VtValue>(Key::RestPoints(path));
	}
	GfMatrix4d& GetGeomBindXform(SdfPath const& path) const {
		return _Get<GfMatrix4d>(Key::GeomBindXform(path));
	}
	VtValue& GetJointIndices(SdfPath const& path) const {
		return _Get<VtValue>(Key::JointIndices(path));
	}
	VtValue& GetJointWeights(SdfPath const& path) const {
		return _Get<VtValue>(Key::JointWeights(path));
	}
	int& GetNumInfluencesPerPoint(SdfPath const& path) const {
		return _Get<int>(Key::NumInfluencesPerPoint(path));
	}
	bool& GetHasConstantInfluences(SdfPath const& path) const {
		return _Get<bool>(Key::HasConstantInfluences(path));
	}
    TfToken& GetSkinningMethod(SdfPath const& path) const {
        return _Get<TfToken>(Key::SkinningMethod(path));
    }
    VtValue& GetSkinningBlendWeights(SdfPath const& path) const {
        return _Get<VtValue>(Key::SkinningBlendWeights(path));
    }
    bool& GetHasConstantSkinningBlendWeights(SdfPath const& path) const {
        return _Get<bool>(Key::HasConstantSkinningBlendWeights(path));
    }
	GfMatrix4d& GetPrimWorldToLocal(SdfPath const& path) const {
		return _Get<GfMatrix4d>(Key::PrimWorldToLocal(path));
	}
	VtValue& GetSkinningXforms(SdfPath const& path) const {
		return _Get<VtValue>(Key::SkinningXforms(path));
	}
	GfMatrix4d& GetSkelLocalToWorld(SdfPath const& path) const {
		return _Get<GfMatrix4d>(Key::SkelLocalToWorld(path));
	}
	//-NV_CHANGE FRZHANG

    bool FindPrimvars(SdfPath const& path, HdPrimvarDescriptorVector* value) const {
        return _Find(Key::Primvars(path), value);
    }

	//+NV_CHANGE FRZHANG : NV GPU Skinning
	bool FindRestPoints(SdfPath const& path, VtValue* value) const {
		return _Find(Key::RestPoints(path), value);
	}
	bool FindGeomBindXform(SdfPath const& path, GfMatrix4d* value) const {
		return _Find(Key::GeomBindXform(path), value);
	}
	bool FindJointIndices(SdfPath const& path, VtValue* value) const {
		return _Find(Key::JointIndices(path), value);
	}
	bool FindJointWeights(SdfPath const& path, VtValue* value) const {
		return _Find(Key::JointWeights(path), value);
	}
	bool FindNumInfluencesPerPoint(SdfPath const& path, int* value) const {
		return _Find(Key::NumInfluencesPerPoint(path), value);
	}
	bool FindHasConstantInfluences(SdfPath const& path, bool* value) const {
		return _Find(Key::HasConstantInfluences(path), value);
	}
    bool FindSkinningMethod(SdfPath const& path, TfToken* value) const {
        return _Find(Key::SkinningMethod(path), value);
    }
    bool FindSkinningBlendWeights(SdfPath const& path, VtValue* value) const {
        return _Find(Key::SkinningBlendWeights(path), value);
    }
    bool FindHasConstantSkinningBlendWeights(SdfPath const& path, bool* value) const {
        return _Find(Key::HasConstantSkinningBlendWeights(path), value);
    }
	bool FindPrimWorldToLocal(SdfPath const& path, GfMatrix4d* value) const {
		return _Find(Key::PrimWorldToLocal(path), value);
	}
	bool FindSkinningXforms(SdfPath const& path, VtValue* value) const {
		return _Find(Key::SkinningXforms(path), value);
	}
	bool FindSkelLocalToWorld(SdfPath const& path, GfMatrix4d* value) const {
		return _Find(Key::SkelLocalToWorld(path), value);
	}

#if 0
    bool ExtractRestPoints(SdfPath const& path, VtValue* value) {
		return _Extract(Key::RestPoints(path), value);
	}
	bool ExtractGeomBindXform(SdfPath const& path, GfMatrix4d* value) {
		return _Extract(Key::GeomBindXform(path), value);
	}
	bool ExtractJointIndices(SdfPath const& path, VtValue* value) {
		return _Extract(Key::JointIndices(path), value);
	}
	bool ExtractJointWeights(SdfPath const& path, VtValue* value) {
		return _Extract(Key::JointWeights(path), value);
	}
	bool ExtractNumInfluencesPerPoint(SdfPath const& path, int* value) {
		return _Extract(Key::NumInfluencesPerPoint(path), value);
	}
	bool ExtractHasConstantInfluences(SdfPath const& path, bool* value) {
		return _Extract(Key::HasConstantInfluences(path), value);
	}
    bool ExtractSkinningMethod(SdfPath const& path, TfToken* value) {
        return _Extract(Key::SkinningMethod(path), value);
    }
    bool ExtractSkinningBlendWeights(SdfPath const& path, VtValue* value) {
        return _Extract(Key::SkinningBlendWeights(path), value);
    }
    bool ExtractHasConstantSkinningBlendWeights(SdfPath const& path, bool* value) {
        return _Extract(Key::HasConstantSkinningBlendWeights(path), value);
    }
	bool ExtractPrimWorldToLocal(SdfPath const& path, GfMatrix4d* value) {
		return _Extract(Key::PrimWorldToLocal(path), value);
	}
	bool ExtractSkinningXforms(SdfPath const& path, VtValue* value) {
		return _Extract(Key::SkinningXforms(path), value);
	}
	bool ExtractSkelLocalToWorld(SdfPath const& path, GfMatrix4d* value) {
		return _Extract(Key::SkelLocalToWorld(path), value);
	}
#endif
	//-NV_CHANGE FRZHANG

private:
    bool _locked;

	//+NV_CHANGE FRZHANG
    typedef _TypedCache<bool> _BoolCache;
    mutable _BoolCache _boolCache;

    typedef _TypedCache<TfToken> _TokenCache;
    mutable _TokenCache _tokenCache;

    typedef _TypedCache<GfMatrix4d> _MatrixCache;
    mutable _MatrixCache _matrixCache;

	typedef _TypedCache<int> _IntCache;
	mutable _IntCache _intCache;
	//-NV_CHANGE FRZHANG

    typedef _TypedCache<VtValue> _ValueCache;
    mutable _ValueCache _valueCache;

    typedef _TypedCache<HdPrimvarDescriptorVector> _PviCache;
    mutable _PviCache _pviCache;

	//+NV_CHANGE FRZHANG
    void _GetCache(_BoolCache **cache) const {
        *cache = &_boolCache;
    }
    void _GetCache(_TokenCache **cache) const {
        *cache = &_tokenCache;
    }
    void _GetCache(_MatrixCache **cache) const {
        *cache = &_matrixCache;
    }
	void _GetCache(_IntCache **cache) const {
		*cache = &_intCache;
	}
	//_NV_CHANGE
    void _GetCache(_ValueCache **cache) const {
        *cache = &_valueCache;
    }

    void _GetCache(_PviCache **cache) const {
        *cache = &_pviCache;
    }
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_USD_IMAGING_USD_IMAGING_PRIMVARDESC_CACHE_H
