#ifndef PXR_IMAGING_HD_NV_HASHMAP_H
#define PXR_IMAGING_HD_NV_HASHMAP_H

// #nv begin #contiguous-values-map
// Not yet ready for PR to public repo
#include <functional>
#include <vector>

#include "pxr/base/tf/diagnostic.h"

PXR_NAMESPACE_OPEN_SCOPE

namespace nv {

    struct HashMapDefaultAlloc
    {
        inline void* operator()(const size_t bytes)
        {
            return std::malloc(bytes);
        }
    };

    struct HashMapDefaultFree
    {
        inline void operator()(void* const ptr)
        {
            std::free(ptr);
        }
    };

    // A hashmap implemented with the following decisions:
    //
    //   * Memory is allocated in a single contiguous buffer so that find operations make linear cache line fetches as much
    //     as possible. This intends to make more easily predictable memory access patterns, and thus, easier hardware-level
    //     prefetch decisions. Similarly, whole-map iteration benefits from the same cache-friendly access patterns.
    //
    //   * Find operations intentionally are coded without range checks on the main loop. This is a tradeoff of speed for
    //     less error-detection in release builds. To help mitigate this, debug builds do track probe counts to validate
    //     we don't exceed the possible length of the hashmap.
    //
    //   * No opinion codified on thread synchronization. It can be used safely if done carefully, but this is not a
    //     guarantee of the implementation.
    //
    //   * No iterators provided. If some batch operation must occur, use the forEach() function provided, which should
    //     suffice. The forEach() method should provide similar performance without the added risk of callers being able to
    //     arbitrarily cache iterators outside the control of the HashMap, its routines, or even its lifetime.
    //
    //   * Deletes copy-constructor and copy-assignment for non-standard-layout mappings. This forces callers to implement
    //     these routines explicitly, favoring the clarity of reading intent in explicit implementation over ambiguity over
    //     compiler selection. Mappings that have standard-layout default to use a memcpy to copy data as fast as possible.
    //
    //   * Implements allocateEntry() method, rather than insert()/emplace() methods mimicing std::unordered_map API. This
    //     does the minimum steps necessary to reserve address space for a key-value mapping, and provides the caller with
    //     the reserved buffer memory address for them to do their own construction, assignment, or initialization routines.
    //     This favors slightly more explicit coding patterns at the caller to force clarity of intent. In particular,
    //     it make more obvious the choice of the caller between construction vs assignment, and copy vs move semantics. It
    //     also offers greater flexibility without sacrificing performance.
    //
    //   * ~HashMap() and clear() operate different depending on if KeyT and ValueT are known to be
    //     std::is_trivially_destructible. If they are, the fastest option is chosen: to deallocate the memory without
    //     iteration or explicitly destruction per-entry. Otherwise, the implementation iterates to non-trivially destruct
    //     each object in-place.

    template<typename KeyT,
        typename ValueT,
        typename HashT = std::hash<KeyT>,
        typename KeyEqualsT = std::equal_to<KeyT>,
        typename AllocT = HashMapDefaultAlloc,
        typename FreeT = HashMapDefaultFree>
        struct HashMap
    {
        // I didn't experiment with this exhaustively, could be tuned better, probably
        static constexpr size_t LOAD_FACTOR_NUMERATOR = 3ull;
        static constexpr size_t LOAD_FACTOR_DENOMENATOR = 4ull;
        static constexpr size_t MIN_INIT_CAPACITY = 4ull;
        static_assert((MIN_INIT_CAPACITY& (MIN_INIT_CAPACITY - 1ull)) == 0, "MIN_INIT_CAPACITY must be a power of two!");

        static constexpr bool KEY_IS_TRIVIALLY_DESTRUCTIBLE = std::is_trivially_destructible<KeyT>::value;
        static constexpr bool VALUE_IS_TRIVIALLY_DESTRUCTIBLE = std::is_trivially_destructible<ValueT>::value;

        enum EntryState : uint8_t
        {
            HASH_MAP_ENTRY_STATE_FREE,
            HASH_MAP_ENTRY_STATE_OCCUPIED,
            HASH_MAP_ENTRY_STATE_DELETED,
        };

        struct EntryT
        {
            EntryState  state;
            KeyT        key;
            ValueT      value;
        };

        static constexpr size_t allocationSize(const size_t capacity);
        static constexpr size_t loadThreshold(const size_t capacity);
        static constexpr size_t inverseLoadThreshold(const size_t capacity);
        static constexpr size_t capacityAdjustedForLoadThreshold(const size_t capacity);

        HashMap(const size_t capacity = 0,
            const HashT& hasher = HashT(),
            const KeyEqualsT& keyEquals = KeyEqualsT(),
            const AllocT& alloc_ = AllocT(),
            const FreeT& free_ = FreeT());

        ~HashMap();

        HashMap(const HashMap& other);
        HashMap& operator=(const HashMap& other);

        HashMap(HashMap&& other) noexcept;
        HashMap& operator=(HashMap&& other) noexcept;

        inline friend void swap(HashMap& a, HashMap& b) noexcept
        {
            using std::swap;
            swap(a.m_hasher, b.m_hasher);
            swap(a.m_keyEquals, b.m_keyEquals);
            swap(a.m_alloc, b.m_alloc);
            swap(a.m_free, b.m_free);
            swap(a.m_occupied, b.m_occupied);
            swap(a.m_deleted, b.m_deleted);
            swap(a.m_capacity, b.m_capacity);
            swap(a.m_loadThreshold, b.m_loadThreshold);
            swap(a.m_mask, b.m_mask);
            swap(a.m_entries, b.m_entries);
        }

        void clear();

        const void* data() const;
        bool empty() const;
        size_t size() const;
        size_t capacty() const;

        void reserve(const size_t capacity);

        bool find(const KeyT& key, ValueT** outValue);
        bool find(const KeyT& key, const ValueT** outValue) const;
        bool exists(const KeyT& key) const;
        bool allocateEntry(KeyT&& key, ValueT** outValue);
        bool allocateEntry(const KeyT& key, ValueT** outValue);

        // Intended to be safe to call during forEach() as it does not invalidate iteration.
        bool freeEntry(const KeyT& key);
        void freeEntryByKeyAddress(const KeyT* const key);
        void freeEntryByValueAddress(const ValueT* const value);

        template<typename CallbackT>
        inline void forEach(CallbackT callback);

        template<typename CallbackT>
        inline void forEach(CallbackT callback) const;

        size_t totalCollisionLength() const;

        ValueT& operator[](KeyT& key)
        {
            ValueT* value;
            allocateEntry(key, &value);
            return *value;
        }

    private:
        size_t hashInternal(const KeyT& key) const;

        void resizeIfNecessary();
        void resize(const size_t nextCapacity);

        void freeEntryInternal(EntryT* const entry);

        bool findExistingOrFirstAvailable(const KeyT& key, EntryT** outEntry);
        bool findExisting(const KeyT& key, EntryT** outEntry);
        bool findExisting(const KeyT& key, const EntryT** outEntry) const;

        HashT m_hasher;
        KeyEqualsT m_keyEquals;
        AllocT m_alloc;
        FreeT m_free;
        size_t m_occupied;
        size_t m_deleted;
        size_t m_capacity;
        size_t m_loadThreshold;
        size_t m_mask;
        EntryT* m_entries;
    };

    inline uint32_t clz64(const uint64_t x)
    {
#if defined(ARCH_OS_WINDOWS)
        unsigned long z;
        return _BitScanReverse64(&z, x) ? 63 - z : 64;
#else
        return  x ? __builtin_clzll(x) : 64;
#endif
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline constexpr size_t HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::allocationSize(const size_t capacity)
    {
        return capacity * sizeof(EntryT);
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline constexpr size_t HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::loadThreshold(const size_t capacity)
    {
        return (capacity * LOAD_FACTOR_NUMERATOR / LOAD_FACTOR_DENOMENATOR);
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline constexpr size_t HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::inverseLoadThreshold(const size_t capacity)
    {
        return (capacity * LOAD_FACTOR_DENOMENATOR / LOAD_FACTOR_NUMERATOR);
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline constexpr size_t HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::capacityAdjustedForLoadThreshold(const size_t capacity)
    {
        // reserves capacity to the nearest power of two that satisfies the load threshhold for the requested capacity

        size_t adjustedCapacity;
        if (capacity && capacity >= loadThreshold(MIN_INIT_CAPACITY))
        {
            // +1 because we want capacity < loadThreshold(adjustedCapacity), not capacity <= loadThreshold(adjustedCapacity)
            adjustedCapacity = 1ull << (64u - clz64(inverseLoadThreshold(capacity + 1) - 1ull));
        }
        else
        {
            adjustedCapacity = MIN_INIT_CAPACITY;
        }
        TF_VERIFY(capacity < loadThreshold(adjustedCapacity));
        TF_VERIFY((adjustedCapacity & (adjustedCapacity - 1ull)) == 0);
        return adjustedCapacity;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::HashMap(const size_t capacity,
        const HashT& hasher,
        const KeyEqualsT& keyEquals,
        const AllocT& alloc_,
        const FreeT& free_)
        : m_hasher(hasher)
        , m_keyEquals(keyEquals)
        , m_alloc(alloc_)
        , m_free(free_)
        , m_occupied(0)
        , m_deleted(0)
    {
        if (capacity)
        {
            const size_t adjustedCapacity = capacityAdjustedForLoadThreshold(capacity);
            const size_t bufSize = allocationSize(adjustedCapacity);
            m_capacity = adjustedCapacity;
            m_loadThreshold = loadThreshold(adjustedCapacity);
            m_mask = adjustedCapacity - 1ull;
            m_entries = (EntryT*)m_alloc(bufSize);
            memset(m_entries, 0, bufSize);
        }
        else
        {
            m_capacity = 0;
            m_loadThreshold = 0;
            m_mask = 0;
            m_entries = nullptr;
        }
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::~HashMap()
    {
        if (m_entries)
        {
            if (!KEY_IS_TRIVIALLY_DESTRUCTIBLE || !VALUE_IS_TRIVIALLY_DESTRUCTIBLE)
            {
                size_t index = 0;
                size_t visited = 0;
                for (; index < m_capacity && visited < m_occupied; ++index)
                {
                    EntryT* const entry = &m_entries[index];
                    if (entry->state == HASH_MAP_ENTRY_STATE_OCCUPIED)
                    {
                        if (!KEY_IS_TRIVIALLY_DESTRUCTIBLE)
                        {
                            entry->key.~KeyT();
                        }

                        if (!VALUE_IS_TRIVIALLY_DESTRUCTIBLE)
                        {
                            entry->value.~ValueT();
                        }

                        TF_VERIFY(visited < m_occupied);
                        ++visited;
                    }
                }
            }

            m_free(m_entries);
            m_entries = nullptr;
            m_occupied = 0;
            m_deleted = 0;
        }
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::HashMap(const HashMap& other)
        : m_hasher(other.m_hasher)
        , m_keyEquals(other.m_keyEquals)
        , m_alloc(other.m_alloc)
        , m_free(other.m_free)
        , m_occupied(other.m_occupied)
        , m_deleted(other.m_deleted)
        , m_capacity(other.m_capacity)
        , m_loadThreshold(other.m_loadThreshold)
        , m_mask(other.m_mask)
    {
        static_assert(std::is_trivially_copyable<EntryT>::value, "Copying of HashMap is only supported for key-value mappings that are use standard-layout classes.");

        const size_t bufSize = allocationSize(m_capacity);
        m_entries = (EntryT*)m_alloc(bufSize);
        memcpy(m_entries, other.m_entries, bufSize);

        TF_VERIFY(m_entries);
        TF_VERIFY(m_capacity);
        TF_VERIFY((m_capacity & (m_capacity - 1ull)) == 0); // assert m_capacity is power of two
        TF_VERIFY(m_occupied < m_capacity);
        TF_VERIFY(m_occupied < m_loadThreshold);
        TF_VERIFY(m_deleted < m_capacity);
        TF_VERIFY(m_deleted < m_loadThreshold);
        TF_VERIFY(m_occupied < m_capacity - m_deleted);
        TF_VERIFY(m_occupied < m_loadThreshold - m_deleted);
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>& HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::operator=(const HashMap& other)
    {
        HashMap tmp(other);
        swap(*this, tmp);
        return *this;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::HashMap(HashMap&& other) noexcept
        : m_hasher(std::move(other.m_hasher))
        , m_keyEquals(std::move(other.m_keyEquals))
        , m_alloc(std::move(other.m_alloc))
        , m_free(std::move(other.m_free))
        , m_occupied(std::move(other.m_occupied))
        , m_deleted(std::move(other.m_deleted))
        , m_capacity(std::move(other.m_capacity))
        , m_loadThreshold(std::move(other.m_loadThreshold))
        , m_mask(std::move(other.m_mask))
        , m_entries(std::move(other.m_entries))
    {
        other.m_entries = nullptr;
        other.clear();
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>& HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::operator=(HashMap&& other) noexcept
    {
        HashMap tmp(std::move(other));
        swap(*this, tmp);
        return *this;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline void HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::clear()
    {
        if (m_entries)
        {
            if (!KEY_IS_TRIVIALLY_DESTRUCTIBLE || !VALUE_IS_TRIVIALLY_DESTRUCTIBLE)
            {
                size_t index = 0;
                size_t visited = 0;
                for (; index < m_capacity && visited < m_occupied; ++index)
                {
                    EntryT* const entry = &m_entries[index];
                    if (entry->state == HASH_MAP_ENTRY_STATE_OCCUPIED)
                    {
                        if (!KEY_IS_TRIVIALLY_DESTRUCTIBLE)
                        {
                            entry->key.~KeyT();
                        }

                        if (!VALUE_IS_TRIVIALLY_DESTRUCTIBLE)
                        {
                            entry->value.~ValueT();
                        }

                        TF_VERIFY(visited < m_occupied);
                        ++visited;
                    }
                    if (entry->state == HASH_MAP_ENTRY_STATE_DELETED)
                    {
                        TF_VERIFY(m_deleted);
                        --m_deleted;
                    }
                    entry->state = HASH_MAP_ENTRY_STATE_FREE;
                }

                // We cannot assert !m_deleted because the variant of clear() for non-trivially destructible mappings that
                // uses the loop above does not guarantee it will visit every deleted entry. It only destructs and frees
                // occupied entries, and stops early, as a shortcut, once all occupied entries have been cleared. If it
                // finds deleted entries along the way, it does mark them as free as a convenience, but not necessarily all
                // deleted entries will be visited and freed this way, which is fine.
                // TF_VERIFY(!m_deleted);
            }
            else
            {
                static_assert(HASH_MAP_ENTRY_STATE_FREE == 0, "memset(0) requires HASH_MAP_ENTRY_STATE_FREE == 0");
                memset(m_entries, 0, allocationSize(m_capacity));
                m_deleted = 0;
            }
        }
        m_occupied = 0;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline const void* HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::data() const
    {
        return m_entries;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline bool HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::empty() const
    {
        return m_occupied == 0;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline size_t HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::size() const
    {
        return m_occupied;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline size_t HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::capacty() const
    {
        return m_capacity;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline void HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::reserve(const size_t capacity)
    {
        const size_t adjustedCapacity = capacityAdjustedForLoadThreshold(capacity);
        if (m_capacity < adjustedCapacity)
        {
            resize(adjustedCapacity);
        }
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline bool HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::find(const KeyT& key, ValueT** outValue)
    {
        EntryT* existing;
        if (findExisting(key, &existing))
        {
            *outValue = &existing->value;
            return true;
        }

        return false;
    }


    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline bool HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::find(const KeyT& key, const ValueT** outValue) const
    {
        const EntryT* existing;
        if (findExisting(key, &existing))
        {
            *outValue = &existing->value;
            return true;
        }

        return false;
    }


    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline bool HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::exists(const KeyT& key) const
    {
        const EntryT* existing;
        if (findExisting(key, &existing))
        {
            return true;
        }

        return false;
    }


    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline bool HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::allocateEntry(KeyT&& key, ValueT** outValue)
    {
        EntryT* availableEntry;

        resizeIfNecessary();

        const bool foundKey = findExistingOrFirstAvailable(key, &availableEntry);
        TF_VERIFY(availableEntry);
        if (!foundKey)
        {
            new (&availableEntry->key) KeyT(std::move(key));
            TF_VERIFY(availableEntry->state != HASH_MAP_ENTRY_STATE_OCCUPIED);
            if (availableEntry->state == HASH_MAP_ENTRY_STATE_DELETED)
            {
                TF_VERIFY(m_deleted);
                --m_deleted;
            }
            availableEntry->state = HASH_MAP_ENTRY_STATE_OCCUPIED;
            *outValue = &availableEntry->value;
            TF_VERIFY(m_occupied < m_capacity);
            TF_VERIFY(m_occupied + 1 > m_occupied);
            ++m_occupied;
            return true;
        }

        *outValue = &availableEntry->value;
        return false;
    }


    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline bool HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::allocateEntry(const KeyT& key, ValueT** outValue)
    {
        EntryT* availableEntry;

        resizeIfNecessary();

        const bool foundKey = findExistingOrFirstAvailable(key, &availableEntry);
        TF_VERIFY(availableEntry);
        if (!foundKey)
        {
            new (&availableEntry->key) KeyT(key);
            TF_VERIFY(availableEntry->state != HASH_MAP_ENTRY_STATE_OCCUPIED);
            if (availableEntry->state == HASH_MAP_ENTRY_STATE_DELETED)
            {
                TF_VERIFY(m_deleted);
                --m_deleted;
            }
            availableEntry->state = HASH_MAP_ENTRY_STATE_OCCUPIED;
            *outValue = &availableEntry->value;
            TF_VERIFY(m_occupied < m_capacity);
            TF_VERIFY(m_occupied + 1 > m_occupied);
            ++m_occupied;
            return true;
        }

        *outValue = &availableEntry->value;
        return false;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline bool HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::freeEntry(const KeyT& key)
    {
        EntryT* existing;
        if (findExisting(key, &existing))
        {
            freeEntryInternal(existing);
            return true;
        }
        else
        {
            return false;
        }
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline void HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::freeEntryByKeyAddress(const KeyT* const key)
    {
        static_assert(!std::is_polymorphic<EntryT>::value, "Unable to freeEntry by key address!");
        constexpr size_t OFFSET = offsetof(EntryT, key);
        EntryT* const entry = (EntryT*)(((uintptr_t)key) - OFFSET);
        freeEntryInternal(entry);
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline void HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::freeEntryByValueAddress(const ValueT* const value)
    {
        static_assert(!std::is_polymorphic<EntryT>::value, "Unable to freeEntry by value address!");
        constexpr size_t OFFSET = offsetof(EntryT, value);
        EntryT* const entry = (EntryT*)(((uintptr_t)value) - OFFSET);
        freeEntryInternal(entry);
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline size_t HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::hashInternal(const KeyT& key) const
    {
        size_t hash = m_hasher(key);

#define HASHMAP_DEFENSIVE_SALT IN_USE
#if defined( HASHMAP_DEFENSIVE_SALT )
        // Apply a defensive salt to the user-calculated hash value. It is unsafe to assume user-provided hashes are good.
        //
        // Kit historically had a problem where std::hash<PathC> caused terrible distributions inside of space-restricted
        // hashmaps. This was primarly because the hash values returned had zero entropy in the lower 8 bits. The higher
        // bits had excellent entropy, though. It is trivial to improve std::hash<PathC> by doing (oldHashValue >> 8).
        // In other words, tossing the bits with zero entropy. This will produce perfectly unique hash value output for
        // every PathC input. However, using this directly in a hash map is still not ideal because, while the hash function
        // has a guarantee on uniqueness, it does not necessarily lend to good distributions in a hash table. Two hash
        // values that are multiples of each other will naturally colliide in any space-restricted hashmap.
        // (Which, realistically, is all real hash maps since hardware memory is not infinite.) Applying a little salt on
        // top of the hash value fixes this distribution problem.
        //
        // This also provides general safety against poorly implemented user-provided hash functions that don't generate
        // unique or well distributed values.
        //
        // Known problematic data sets:
        // - PathC (interned SdfPaths)
        // - TokenC (interned TfTokens)
        //
        // Salt techniques tried:
        // - H3_XX64 (xxhash):
        //     - good distribution
        //     - too slow
        // - H3_XX64 (xxhash) with custom seeds:
        //     - no seed performed better than the xxhash default secret
        // - Custom H3_XX64 implementation specialized for aligned 64-bit keys:
        //     - methematically identical distribution to H3_XX64
        //     - 2x faster performance than official implementation
        // - Multiply by a prime
        //     - best distribution so far
        //     - best speed so far (3x faster than custom H3_XX64)
        //
        // TODO: A fun intern experiment would be to investigate our various omniverse hash functions for distribution and
        // speed. And also investigate alternative defensive salting techniques.

        return hash * 48271ull;
#else // #if USING( HASHMAP_DEFENSIVE_SALT )
        return hash;
#endif // #if USING( HASHMAP_DEFENSIVE_SALT )
#undef HASHMAP_DEFENSIVE_SALT
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline void HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::resizeIfNecessary()
    {
        if (!m_entries)
        {
            resize(MIN_INIT_CAPACITY);
        }
        else
        {
            TF_VERIFY(m_capacity);
            if (m_occupied >= m_loadThreshold)
            {
                const size_t nextCapacity = m_capacity * 2;
                TF_VERIFY(nextCapacity > m_capacity); // protect against overflow
                resize(nextCapacity);
            }
            else if (m_occupied >= m_loadThreshold - m_deleted)
            {
                resize(m_capacity); // rehashes table to clear DELETED entries
            }
        }

        TF_VERIFY(m_entries);
        TF_VERIFY(m_capacity);
        TF_VERIFY((m_capacity & (m_capacity - 1)) == 0);
        TF_VERIFY(m_occupied < m_capacity);
        TF_VERIFY(m_occupied < m_loadThreshold);
        TF_VERIFY(m_deleted < m_capacity);
        TF_VERIFY(m_deleted < m_loadThreshold);
        TF_VERIFY(m_occupied < m_capacity - m_deleted);
        TF_VERIFY(m_occupied < m_loadThreshold - m_deleted);
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline void HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::resize(const size_t nextCapacity)
    {
        TF_VERIFY(m_occupied < loadThreshold(nextCapacity));
        TF_VERIFY((nextCapacity & (nextCapacity - 1)) == 0);

        HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT> tmp(nextCapacity, m_hasher, m_keyEquals, m_alloc, m_free);

        size_t index = 0;
        size_t visited = 0;
        for (; index < m_capacity && visited < m_occupied; ++index)
        {
            EntryT* const entry = &m_entries[index];
            if (entry->state == HASH_MAP_ENTRY_STATE_OCCUPIED)
            {
                ValueT* tmpV;
                tmp.allocateEntry(std::move(entry->key), &tmpV);
                new (tmpV) ValueT(std::move(entry->value));

                TF_VERIFY(visited < m_occupied);
                ++visited;
            }
        }

        TF_VERIFY(m_occupied == tmp.m_occupied);
        TF_VERIFY(!tmp.m_deleted);
        using std::swap;
        swap(*this, tmp);
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline void HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::freeEntryInternal(EntryT* const entry)
    {
        TF_VERIFY(entry);
        TF_VERIFY(entry->state == HASH_MAP_ENTRY_STATE_OCCUPIED);

        if (!KEY_IS_TRIVIALLY_DESTRUCTIBLE)
        {
            entry->key.~KeyT();
        }
        if (!VALUE_IS_TRIVIALLY_DESTRUCTIBLE)
        {
            entry->value.~ValueT();
        }

        entry->state = HASH_MAP_ENTRY_STATE_DELETED;
        TF_VERIFY(m_occupied);
        TF_VERIFY(m_occupied - 1 < m_occupied);
        --m_occupied;
        ++m_deleted;
        TF_VERIFY(m_deleted);

        resizeIfNecessary();
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    template<typename CallbackT>
    inline void HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::forEach(CallbackT callback)
    {

        size_t index = 0;
        size_t visited = 0;
        const size_t size_captured = m_occupied;
        for (; index < m_capacity && visited < size_captured; ++index)
        {
            if (m_entries[index].state == HASH_MAP_ENTRY_STATE_OCCUPIED)
            {
                callback(m_entries[index].key, m_entries[index].value);

                TF_VERIFY(visited < size_captured);
                ++visited;
            }
        }
    }


    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    template<typename CallbackT>
    inline void HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::forEach(CallbackT callback) const
    {
        size_t index = 0;
        size_t visited = 0;
        const size_t size_captured = m_occupied;
        for (; index < m_capacity && visited < size_captured; ++index)
        {
            if (m_entries[index].state == HASH_MAP_ENTRY_STATE_OCCUPIED)
            {
                callback(m_entries[index].key, m_entries[index].value);

                TF_VERIFY(visited < size_captured);
                ++visited;
            }
        }
    }


    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline size_t HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::totalCollisionLength() const
    {
        size_t len = 0;

        if (m_entries)
        {
            size_t index = 0;
            size_t visited = 0;
            const size_t size_captured = m_occupied;
            for (; index < m_capacity && visited < size_captured; ++index)
            {
                const EntryT* const probe = &m_entries[index];
                if (probe->state == HASH_MAP_ENTRY_STATE_OCCUPIED)
                {
                    const EntryT* const natural = &m_entries[hashInternal(probe->key) & m_mask];
                    len += (size_t)((natural <= probe) ? (probe - natural) : ((probe + m_capacity) - natural));

                    TF_VERIFY(visited < size_captured);
                    ++visited;
                }
            }
        }

        return len;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline bool HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::findExistingOrFirstAvailable(const KeyT& key, EntryT** outEntry)
    {
        EntryT* probe;
        size_t probeIdx;

#if defined( ASSERTS )
        size_t probes = 0;
#endif // #if USING( ASSERTS )
        probeIdx = hashInternal(key) & m_mask;

        TF_VERIFY(m_entries);
        TF_VERIFY(m_occupied < m_capacity); // otherwise we infinite loop
        while (1)
        {
            TF_VERIFY(probeIdx < m_capacity);
            probe = &m_entries[probeIdx];

            if (probe->state == HASH_MAP_ENTRY_STATE_FREE)
            {
                *outEntry = probe;
                return false;
            }
            else if (probe->state == HASH_MAP_ENTRY_STATE_OCCUPIED)
            {
                if (m_keyEquals(probe->key, key))
                {
                    *outEntry = probe;
                    return true;
                }
            }
            else if (probe->state == HASH_MAP_ENTRY_STATE_DELETED)
            {
                *outEntry = probe;
                return false;
            }

            probeIdx = (probeIdx + 1) & m_mask;
#if defined( ASSERTS )
            ++probes;
            TF_VERIFY(probes < m_capacity);
#endif // #if USING( ASSERTS )
        }

        TF_VERIFY(false && "unreachable code");
        return false;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline bool HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::findExisting(const KeyT& key, EntryT** outEntry)
    {
        if (!m_occupied)
        {
            return false;
        }

        EntryT* probe;
        size_t probeIdx;

#if defined( ASSERTS )
        size_t probes = 0;
#endif // #if USING( ASSERTS )
        probeIdx = hashInternal(key) & m_mask;

        TF_VERIFY(m_occupied < m_capacity); // otherwise we infinite loop
        while (1)
        {
            TF_VERIFY(probeIdx < m_capacity);
            probe = &m_entries[probeIdx];

            if (probe->state == HASH_MAP_ENTRY_STATE_FREE)
            {
                return false;
            }
            else if (probe->state == HASH_MAP_ENTRY_STATE_OCCUPIED)
            {
                if (m_keyEquals(probe->key, key))
                {
                    *outEntry = probe;
                    return true;
                }
            }
            else
            {
                // skip
                TF_VERIFY(probe->state == HASH_MAP_ENTRY_STATE_DELETED);
            }

            probeIdx = (probeIdx + 1) & m_mask;
#if defined( ASSERTS )
            ++probes;
            TF_VERIFY(probes < m_capacity);
#endif // #if USING( ASSERTS )
        }

        TF_VERIFY(false && "unreachable code");
        return false;
    }


    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqualsT, typename AllocT, typename FreeT>
    inline bool HashMap<KeyT, ValueT, HashT, KeyEqualsT, AllocT, FreeT>::findExisting(const KeyT& key, const EntryT** outEntry) const
    {
        return const_cast<HashMap*>(this)->findExisting(key, const_cast<EntryT**>(outEntry));
    }

    template<class KeyType, class ValueType, class HashFn = std::hash<KeyType>>
    struct contiguous_values_map
    {
    private:
        HashMap<KeyType, size_t, HashFn> valueIndices;
        std::vector<std::pair<ValueType, bool>> values;
        size_t capacity = 5000;
    public:
        contiguous_values_map() { values.reserve(capacity); }
        ValueType& operator[](const KeyType& key)
        {
            size_t* idx;
            if (valueIndices.find(key, &idx))
                return values[*idx].first;
            valueIndices.allocateEntry(key, &idx);

            size_t &newIdx = *idx;
            newIdx = values.size(); // new idx is end of values array
            if( newIdx == capacity ) // reserve more space if necessary
            {
                capacity *= 2;
                values.reserve(capacity);
            }
            // add new element
            values.resize(newIdx + 1);
            
            auto &pair = values[newIdx];
            pair.second = true;
            return pair.first;
        }
        bool find(const KeyType& key, ValueType** value)
        {
            size_t* idx;
            if (!valueIndices.find(key, &idx))
                return false;
            *value = &values[*idx].first;
            return true;
        }
        bool find(const KeyType& key, const ValueType** value) const
        {
            const size_t* idx;
            if (!valueIndices.find(key, &idx))
                return false;
            *value = &values[*idx].first;
            return true;
        }
        void erase(const KeyType& key)
        {
            size_t* idx;
            if (valueIndices.find(key, &idx))
            {
                values[*idx].second = false;
                valueIndices.freeEntry(key);
            }
        }
        void for_each_value(std::function<void(ValueType&)> fn, bool parallel = false)
        {
            for (size_t i = 0; i < values.size(); ++i)
                if( values[i].second )
                    fn(values[i].first);
        }
    };


} // namespace nv

PXR_NAMESPACE_CLOSE_SCOPE

#endif