#ifndef COREOS_MLA_HASH_MAP_H
#define COREOS_MLA_HASH_MAP_H

#include <iterator>

#include "../mla_data_types.h"
#include "mla_array_list.h"

#define mla_hash_map_template_full class TKey, class TValue, class Hasher, class TKeyInit = mla_default_init(TKey), class TValueInit = mla_default_init(TValue)
#define mla_hash_map_template class TKey, class TValue, class TKeyInit = mla_default_init(TKey), class TValueInit = mla_default_init(TValue)

#define mla_hash_map_t_param TKey, TValue, TKeyInit, TValueInit
#define mla_hash_map_t_param_full TKey, TValue, Hasher, TKeyInit, TValueInit

#define CONST_mla_hash_map_default_bucket_size 8
#define CONST_mla_hash_map_default_load_factor 0.75
#define CONST_mla_hash_map_shrink_load_factor 0.25

enum mla_hash_map_bucket_state {
    BUCKET_EMPTY,
    BUCKET_OCCUPIED,
    BUCKET_DELETED
};


template < mla_hash_map_template >
struct mla_hash_map_bucket_t {
    TKey key;
    TValue value;
    mla_hash_map_bucket_state state;
};

template < mla_hash_map_template >
struct mla_hash_map_bucket_t_initializer {
    static mla_hash_map_bucket_t<mla_hash_map_t_param> init() {
        return {
                TKeyInit::init(),
                TValueInit::init(),
                BUCKET_EMPTY
        };
    }
};

template < mla_hash_map_template_full >
struct mla_hash_map_t {
    mla_size_t size;
    mla_size_t bucketCount;
    mla_float_t loadFactor;
    mla_array_list_t<mla_hash_map_bucket_t<mla_hash_map_t_param>, mla_hash_map_bucket_t_initializer<mla_hash_map_t_param>> buckets;
};

template < mla_hash_map_template_full >
mla_hash_map_t<mla_hash_map_t_param_full> mla_hash_map(mla_size_t bucketCount = CONST_mla_hash_map_default_bucket_size, mla_float_t loadFactor = CONST_mla_hash_map_default_load_factor) {


    auto array =  mla_array_list<mla_hash_map_bucket_t<mla_hash_map_t_param>, mla_hash_map_bucket_t_initializer<mla_hash_map_t_param>>(bucketCount);
    bucketCount = mla_array_list_capacity(array);

    return {
        0, // Initialize size to 0
        bucketCount, // Set the number of buckets
        loadFactor, // Set the load factor
        array // Initialize the buckets with the specified bucket count
    };

}


template < mla_hash_map_template_full >
inline mla_hash_map_t<mla_hash_map_t_param_full> mla_hash_map_empty() {

    return {
        0, // Initialize size to 0
        0, // Set the number of buckets to 0
        0,
        mla_array_list_empty<mla_hash_map_bucket_t<mla_hash_map_t_param>, mla_hash_map_bucket_t_initializer<mla_hash_map_t_param>>()
    };

}

template < mla_hash_map_template_full >
mla_size_t mla_hash_map_size(mla_hash_map_t<mla_hash_map_t_param_full> &map) {
    return map.size; // Return the current size of the hash map
}

template < mla_hash_map_template_full >
void mla_hash_map_resize(mla_hash_map_t<mla_hash_map_t_param_full>& map, mla_size_t newBucketCount) {

    if (newBucketCount < CONST_mla_hash_map_default_bucket_size) {
        newBucketCount = CONST_mla_hash_map_default_bucket_size;
    }

    auto newBuckets = mla_array_list<mla_hash_map_bucket_t<mla_hash_map_t_param>,
            mla_hash_map_bucket_t_initializer<mla_hash_map_t_param>>(newBucketCount);

    mla_size_t newCapacity = mla_array_list_capacity(newBuckets);
    if (newCapacity < newBucketCount) {
        mla_array_list_destroy(newBuckets);
        return;
    }

    newBucketCount = newCapacity;

    for (mla_size_t i = 0; i < map.bucketCount; ++i) {
        auto& oldBucket = mla_array_list_get_unsafe(map.buckets, i);
        if (oldBucket.state == BUCKET_OCCUPIED) {
            mla_size_t newIndex = Hasher::hash(oldBucket.key) % newBucketCount;
            while (true) {
                auto& newBucket = mla_array_list_get_unsafe(newBuckets, newIndex);
                if (newBucket.state == BUCKET_EMPTY) {
                    newBucket = oldBucket;
                    break;
                }
                newIndex = (newIndex + 1) % newBucketCount;
            }
        }
    }

    mla_array_list_destroy(map.buckets);
    map.buckets = newBuckets;
    map.bucketCount = newBucketCount;
}


enum mla_hash_map_push_result {
    MLA_HASH_MAP_PUSH_REPLACED,
    MLA_HASH_MAP_PUSH_ADDED,
    MLA_HASH_MAP_PUSH_ERROR
};

template < mla_hash_map_template_full >
mla_hash_map_push_result mla_hash_map_push(mla_hash_map_t<mla_hash_map_t_param_full> &map, const TKey &key, const TValue &value) {

    if (map.bucketCount > 0) {
        mla_float_t currentLoadFactor = (mla_float_t)map.size / (mla_float_t)map.bucketCount;
        if (currentLoadFactor > map.loadFactor) {
            mla_hash_map_resize(map, map.bucketCount * 2);
        }
    }

    if (map.bucketCount == 0) {
        map.loadFactor = CONST_mla_hash_map_default_load_factor;
        map.buckets = mla_array_list<mla_hash_map_bucket_t<mla_hash_map_t_param>, mla_hash_map_bucket_t_initializer<mla_hash_map_t_param>>(CONST_mla_hash_map_default_bucket_size);
        map.bucketCount = mla_array_list_capacity(map.buckets);

        if (map.bucketCount == 0) {
            return MLA_HASH_MAP_PUSH_ERROR;
        }
    }

    mla_size_t index = Hasher::hash(key) % map.bucketCount;
    mla_size_t originalIndex = index;
    mla_size_t deletedIndex = -1;

    do {
        auto& bucket = mla_array_list_get_unsafe(map.buckets, index);
        if (bucket.state == BUCKET_OCCUPIED) {
            if (bucket.key == key) {
                bucket.value = value;
                return MLA_HASH_MAP_PUSH_REPLACED;
            }
        } else if (bucket.state == BUCKET_DELETED) {
            if (deletedIndex == -1) {
                deletedIndex = index;
            }
        } else { // BUCKET_EMPTY
            if (deletedIndex != -1) {
                index = deletedIndex;
            }
            auto& newBucket = mla_array_list_get_unsafe(map.buckets, index);
            newBucket.key = key;
            newBucket.value = value;
            newBucket.state = BUCKET_OCCUPIED;
            map.size++;
            return MLA_HASH_MAP_PUSH_ADDED;
        }

        index = (index + 1) % map.bucketCount;

    } while (index != originalIndex);

    // If we are here, the map is full and we have looped back
    // This part should ideally be preceded by a resize, controlled by the load factor,
    // but if resizing fails or is not triggered, this indicates an error state.
    return MLA_HASH_MAP_PUSH_ADDED;

}

template < mla_hash_map_template_full >
mla_bool_t mla_hash_map_contains(const mla_hash_map_t<mla_hash_map_t_param_full> &map, const TKey &key) {

    if (map.size == 0) {
        return false; // Empty map cannot contain any keys
    }

    mla_int32_t index = Hasher::hash(key) % map.bucketCount;
    mla_int32_t originalIndex = index;

    do {
        const auto& bucket = mla_array_list_get_unsafe(map.buckets, index);

        if (bucket.state == BUCKET_OCCUPIED) {
            if (bucket.key == key) {
                return true; // Key found
            }
        } else if (bucket.state == BUCKET_EMPTY) {
            return false; // Key not found
        }

        index = (index + 1) % map.bucketCount;

    } while (index != originalIndex);

    return false; // Key not found after a full loop
}

template < mla_hash_map_template_full >
mla_bool_t mla_hash_map_remove(mla_hash_map_t<mla_hash_map_t_param_full> &map, const TKey &key) {

    if (map.size == 0) {
        return false; // Empty map cannot contain any keys
    }

    mla_int32_t index = Hasher::hash(key) % map.bucketCount;
    mla_int32_t originalIndex = index;

    do {
        auto& bucket = mla_array_list_get_unsafe(map.buckets, index);

        if (bucket.state == BUCKET_OCCUPIED) {
            if (bucket.key == key) {
                bucket.state = BUCKET_DELETED;
                map.size--;
                if (map.bucketCount > CONST_mla_hash_map_default_bucket_size) {
                    mla_float_t currentLoadFactor = (mla_float_t)map.size / (mla_float_t)map.bucketCount;
                    if (currentLoadFactor < CONST_mla_hash_map_shrink_load_factor) {
                        mla_hash_map_resize(map, map.bucketCount / 2);
                    }
                }
                return true;
            }
        } else if (bucket.state == BUCKET_EMPTY) {
            return false;
        }

        index = (index + 1) % map.bucketCount;

    } while (index != originalIndex);

    return false;
}

template< mla_hash_map_template_full >
mla_array_list_t<TKey, TKeyInit> mla_hash_map_keys(const mla_hash_map_t<mla_hash_map_t_param_full> &map) {

    mla_array_list_t<TKey, TKeyInit> keys = mla_array_list<TKey, TKeyInit>(map.size);

    for (mla_size_t i = 0; i < map.bucketCount; ++i) {
        auto& bucket = mla_array_list_get_unsafe(map.buckets, i);
        if (bucket.state == BUCKET_OCCUPIED) {
            mla_array_list_add(keys, bucket.key);
        }
    }

    return keys;

}

template < mla_hash_map_template_full >
mla_bool_t mla_hash_map_get(const mla_hash_map_t<mla_hash_map_t_param_full> &map, const TKey &key, TValue &value) {

    if (map.size == 0) {
        value = TValueInit::init(); // Set value to default if map is empty
        return false; // Empty map cannot contain any keys
    }

    mla_int32_t index = Hasher::hash(key) % map.bucketCount;
    mla_int32_t originalIndex = index;

    do {
        const auto& bucket = mla_array_list_get_unsafe(map.buckets, index);

        if (bucket.state == BUCKET_OCCUPIED) {
            if (bucket.key == key) {
                value = bucket.value;
                return true;
            }
        } else if (bucket.state == BUCKET_EMPTY) {
            value = TValueInit::init();
            return false;
        }

        index = (index + 1) % map.bucketCount;

    } while (index != originalIndex);

    value = TValueInit::init();
    return false;
}

template < mla_hash_map_template_full >
TValue* mla_hash_map_get_ref (mla_hash_map_t<mla_hash_map_t_param_full> &map, const TKey &key) {

    if (map.size == 0) {
        return nullptr; // Empty map cannot contain any keys
    }

    mla_int32_t index = Hasher::hash(key) % map.bucketCount;
    mla_int32_t originalIndex = index;

    do {
        auto& bucket = mla_array_list_get_unsafe(map.buckets, index);

        if (bucket.state == BUCKET_OCCUPIED) {
            if (bucket.key == key) {
                return &bucket.value;
            }
        } else if (bucket.state == BUCKET_EMPTY) {
            return nullptr;
        }

        index = (index + 1) % map.bucketCount;

    } while (index != originalIndex);

    return nullptr;
}

template < mla_hash_map_template_full >
void mla_hash_map_clear(mla_hash_map_t<mla_hash_map_t_param_full> &map) {

    if (map.size == 0) {
        return; // Nothing to clear
    }

    for (mla_size_t i = 0; i < map.bucketCount; ++i) {
        auto& bucket = mla_array_list_get_unsafe(map.buckets, i);
        bucket.state = BUCKET_EMPTY;
        bucket.key = TKeyInit::init();
        bucket.value = TValueInit::init();
    }

    map.size = 0; // Reset the size of the hash map

}


#endif
