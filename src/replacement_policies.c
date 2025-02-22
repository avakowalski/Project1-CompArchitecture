//
// This file contains all of the implementations of the replacement_policy
// constructors from the replacement_policies.h file.
//
// It also contains stubs of all of the functions that are added to each
// replacement_policy struct at construction time.
//
// ============================================================================
// NOTE: It is recommended that you read the comments in the
// replacement_policies.h file for further context on what each function is
// for.
// ============================================================================
//

#include "replacement_policies.h"

// LRU Replacement Policy
// ============================================================================
// TODO feel free to create additional structs/enums as necessary

void lru_cache_access(struct replacement_policy *replacement_policy,
                      struct cache_system *cache_system, uint32_t set_idx, uint32_t tag)
{
    // TODO update the LRU replacement policy state given a new memory access
}

// this function helps select the least recently used line for eviction 
uint32_t lru_eviction_index(struct replacement_policy *replacement_policy,
                            struct cache_system *cache_system, uint32_t set_idx)
{
    struct lru_metadata *metadata = (struct lru_metadata *)replacement_policy->data;

    // The least recently used index is the first item in the list
    return metadata->lru_list[set_idx][0];
}

void lru_replacement_policy_cleanup(struct replacement_policy *replacement_policy)
{
    // TODO cleanup any additional memory that you allocated in the
    // lru_replacement_policy_new function.
}

struct replacement_policy *lru_replacement_policy_new(uint32_t sets, uint32_t associativity)
{
    struct replacement_policy *lru_rp = calloc(1, sizeof(struct replacement_policy));
    lru_rp->cache_access = &lru_cache_access;
    lru_rp->eviction_index = &lru_eviction_index;
    lru_rp->cleanup = &lru_replacement_policy_cleanup;

    // TODO allocate any additional memory to store metadata here and assign to
    // lru_rp->data.

    return lru_rp;
}

// RAND Replacement Policy
// ============================================================================
void rand_cache_access(struct replacement_policy *replacement_policy,
                       struct cache_system *cache_system, uint32_t set_idx, uint32_t tag)
{
    // TODO update the RAND replacement policy state given a new memory access
}

uint32_t rand_eviction_index(struct replacement_policy *replacement_policy,
                             struct cache_system *cache_system, uint32_t set_idx)
{
    //this randomly picks an index within the set 
    return rand() % cache_system->associativity;
}

void rand_replacement_policy_cleanup(struct replacement_policy *replacement_policy)
{
    // TODO cleanup any additional memory that you allocated in the
    // rand_replacement_policy_new function.
}

struct replacement_policy *rand_replacement_policy_new(uint32_t sets, uint32_t associativity)
{
    // Seed randomness
    srand(time(NULL));

    struct replacement_policy *rand_rp = malloc(sizeof(struct replacement_policy));
    rand_rp->cache_access = &rand_cache_access;
    rand_rp->eviction_index = &rand_eviction_index;
    rand_rp->cleanup = &rand_replacement_policy_cleanup;

    // TODO allocate any additional memory to store metadata here and assign to
    // rand_rp->data.

    return rand_rp;
}

// LRU_PREFER_CLEAN Replacement Policy
// ============================================================================
void lru_prefer_clean_cache_access(struct replacement_policy *replacement_policy,
                                   struct cache_system *cache_system, uint32_t set_idx,
                                   uint32_t tag)
{
    struct lru_metadata *metadata = (struct lru_metadata *)replacement_policy->data;  // ✅ Cast here
    metadata->lru_list[set_idx][0] = tag;
}

//this function prefers evicting a "clean" cache line 
//if all lines are dirty, evict the least recently used cache line 
uint32_t lru_prefer_clean_eviction_index(struct replacement_policy *replacement_policy,
                                         struct cache_system *cache_system, uint32_t set_idx)
{
    struct lru_metadata *metadata = (struct lru_metadata *)replacement_policy->data;  // ✅ Cast here

    // Try to find a clean cache line to evict
    for (int i = 0; i < cache_system->associativity; i++) {
        uint32_t index = metadata->lru_list[set_idx][i];  // ✅ Now works!
        if (cache_system->cache_lines[set_idx * cache_system->associativity + index].status != MODIFIED) {
            return index;
        }
    }
    // If no clean cache line, fall back to normal LRU
    return metadata->lru_list[set_idx][0];
}

void lru_prefer_clean_replacement_policy_cleanup(struct replacement_policy *replacement_policy)
{
    struct lru_metadata *metadata = (struct lru_metadata *)replacement_policy->data;  // ✅ Cast

    for (uint32_t i = 0; i < sizeof(metadata->lru_list) / sizeof(metadata->lru_list[0]); i++) {
        free(metadata->lru_list[i]);
    }
    free(metadata->lru_list);
    free(metadata);
    free(replacement_policy);
}

struct replacement_policy *lru_prefer_clean_replacement_policy_new(uint32_t sets,
                                                                   uint32_t associativity)
{
    struct replacement_policy *lru_prefer_clean_rp = malloc(sizeof(struct replacement_policy));
    lru_prefer_clean_rp->cache_access = &lru_prefer_clean_cache_access;
    lru_prefer_clean_rp->eviction_index = &lru_prefer_clean_eviction_index;
    lru_prefer_clean_rp->cleanup = &lru_prefer_clean_replacement_policy_cleanup;

    // TODO allocate any additional memory to store metadata here and assign to
    

    struct lru_metadata *metadata = calloc(1, sizeof(struct lru_metadata));
    metadata->lru_list = malloc(sets * sizeof(uint32_t *));
    for (uint32_t i = 0; i < sets; i++) {
        metadata->lru_list[i] = malloc(associativity * sizeof(uint32_t));
    }

    //Assign metadata to the replacement policy
    lru_prefer_clean_rp->data = metadata;

    return lru_prefer_clean_rp;
}
