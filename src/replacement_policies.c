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

//this function is the function called whenever a cahce line is acessed 
void lru_cache_access(struct replacement_policy *replacement_policy,
                      struct cache_system *cache_system, uint32_t set_idx, uint32_t tag)
{
    //gets the metadata for the LRU policy 
    struct lru_metadata *metadata = (struct lru_metadata *)replacement_policy->data;
    if (!metadata || set_idx >= metadata->num_sets) return;

    uint32_t *lru_list = metadata->lru_list[set_idx];
    int accessed_index = -1;

    // Find the which cache line (identified by its index) in the cache system, contains the requested tag
    for (int i = 0; i < cache_system->associativity; i++) {
        if (cache_system->cache_lines[set_idx * cache_system->associativity + i].tag == tag) {
            accessed_index = i;
            break;
        }
    }

    if (accessed_index == -1) return; // the tag is not found, womp womp

    // finding the position of that cache line's index in the LRU list
    int lru_position = -1;
    for (int i = 0; i < cache_system->associativity; i++) {
        if (lru_list[i] == accessed_index) {
            lru_position = i;
            break;
        }
    }

    if (lru_position == -1) return; // the index not in LRU list (this shouldn't happen? but just in case)

   
    uint32_t accessed_value = lru_list[lru_position];
    // shift all elements right up to the position
    for (int i = lru_position; i > 0; i--) {
        lru_list[i] = lru_list[i - 1];
    }
    // move accessed cache line's index to front of the LRU list (would be position 0)
    //this will help the LRU list w most recent used at front and least at the back 
    lru_list[0] = accessed_value;
}



// this function helps select the least recently used line for eviction 
uint32_t lru_eviction_index(struct replacement_policy *replacement_policy,
                            struct cache_system *cache_system, uint32_t set_idx) {
    
    struct lru_metadata *metadata = (struct lru_metadata *)replacement_policy->data;
    if (!metadata || set_idx >= metadata->num_sets) {
        fprintf(stderr, "ERROR: Invalid set index %u\n", set_idx);
        exit(1);
    }
    //retrieves the LRU list for the set
    uint32_t *lru_list = metadata->lru_list[set_idx];

    //  db print, before eviction
    //printf("[DEBUG] LRU List Before Eviction (Set %u): ", set_idx);
    // for (int i = 0; i < cache_system->associativity; i++) {
    //     printf("%u ", lru_list[i]);
    // }
    // printf("\n");

    //returns the index of the LRU cahce line which hopefully is stored at the end of the LRU list
    uint32_t lru_index = lru_list[cache_system->associativity - 1];

    // makes sure the index is valid
    if (lru_index >= cache_system->associativity) {
        fprintf(stderr, "ERROR: Invalid LRU index %u for set %u\n", lru_index, set_idx);
        return 0;  // Return a valid default instead of crashing
    }

    return lru_index;
}

//frees all mem allocated for the LRU policy 
void lru_replacement_policy_cleanup(struct replacement_policy *rp) {
    if (!rp) return;

    struct lru_metadata *metadata = (struct lru_metadata *)rp->data;
    if (metadata) {
        if (metadata->lru_list) {
            for (uint32_t i = 0; i < metadata->num_sets; i++) {
                // frees each set's LRU list
                free(metadata->lru_list[i]);  
            }
            // prevents NULL access
            if (!metadata || !metadata->lru_list) return;  
            // frees the LRU list array
            free(metadata->lru_list);  
        }
        //frees the metadata structure
        free(metadata);  
    }

    // DO NOT free rp here, let cache_system_cleanup handle it,, learned this the hard way 
}
//constructor creates a new LRU replacement policy 
struct replacement_policy *lru_replacement_policy_new(uint32_t sets, uint32_t associativity) {
    struct replacement_policy *lru_rp = malloc(sizeof(struct replacement_policy));
    if (!lru_rp) return NULL;
    //sets up the function pointers for access, eviction, and cleanup 
    lru_rp->cache_access = &lru_cache_access;
    lru_rp->eviction_index = &lru_eviction_index;
    lru_rp->cleanup = &lru_replacement_policy_cleanup;

    //allocates and initializes the metadata structure 
    struct lru_metadata *metadata = malloc(sizeof(struct lru_metadata));
    if (!metadata) {
        free(lru_rp);
        return NULL;
    }

    metadata->num_sets = sets;
    metadata->lru_list = malloc(sets * sizeof(uint32_t *));
    if (!metadata->lru_list) {
        free(metadata);
        free(lru_rp);
        return NULL;
    }

    // declaring `i` before the loop
    uint32_t i, j;

    //creation of an LRU list for each set 
    for (i = 0; i < sets; i++) {
        metadata->lru_list[i] = malloc(associativity * sizeof(uint32_t));
        if (!metadata->lru_list[i]) {
            for (j = 0; j < i; j++) free(metadata->lru_list[j]);
            free(metadata->lru_list);
            free(metadata);
            free(lru_rp);
            return NULL;
        }

        // initializes each LRU list w indices 0 to associativity-1
        for (j = 0; j < associativity; j++) {
            metadata->lru_list[i][j] = j;
        }

        // use `i` after declaring it
        //printf("[DEBUG] LRU Set %u initialized: ", i);
        // for (j = 0; j < associativity; j++) {
        //     printf("%u ", metadata->lru_list[i][j]);
        // }
        // printf("\n");
    }
    //returns the configured replacement policy
    lru_rp->data = metadata;
    return lru_rp;
}


// RAND Replacement Policy
// ============================================================================

//called when a cache line is accessed 
void rand_cache_access(struct replacement_policy *replacement_policy,
                       struct cache_system *cache_system, uint32_t set_idx, uint32_t tag)
{
    // RAND does NOT track accesses, so this function does nothing.
    return;

}

uint32_t rand_eviction_index(struct replacement_policy *replacement_policy,
                             struct cache_system *cache_system, uint32_t set_idx)
{
    //this randomly picks an index within the set 
    // Use a more robust random number generation
    return (uint32_t)(rand() % cache_system->associativity);
}

void rand_replacement_policy_cleanup(struct replacement_policy *replacement_policy)
{
    if (!replacement_policy) return;  // Prevent NULL pointer issues

    if (replacement_policy->data) {
        printf("[DEBUG] Freeing rand_replacement_policy data\n");
        free(replacement_policy->data);
        replacement_policy->data = NULL;  //Prevent double free
    }
}





struct replacement_policy *rand_replacement_policy_new(uint32_t sets, uint32_t associativity)
{
    srand(time(NULL));

    struct replacement_policy *rand_rp = malloc(sizeof(struct replacement_policy));
    if (!rand_rp) return NULL;

    rand_rp->cache_access = &rand_cache_access;
    rand_rp->eviction_index = &rand_eviction_index;
    rand_rp->cleanup = &rand_replacement_policy_cleanup;

    rand_rp->data = NULL;  //Initialize data to prevent uninitialized use

    return rand_rp;
}


// LRU_PREFER_CLEAN Replacement Policy
// ============================================================================
void lru_prefer_clean_cache_access(struct replacement_policy *replacement_policy,
                                  struct cache_system *cache_system, uint32_t set_idx,
                                  uint32_t tag)
{
    struct lru_metadata *metadata = (struct lru_metadata *)replacement_policy->data;  
    if (!metadata) return;

    uint32_t *lru_list = metadata->lru_list[set_idx];
    int accessed_index = -1;

    // Step 1: Find the cache line with this tag in the cache system
    for (int i = 0; i < cache_system->associativity; i++) {
        if (cache_system->cache_lines[set_idx * cache_system->associativity + i].tag == tag) {
            accessed_index = i;
            break;
        }
    }

    if (accessed_index == -1) return; // Tag not found

    // Step 2: Find this index in the LRU list
    int lru_position = -1;
    for (int i = 0; i < cache_system->associativity; i++) {
        if (lru_list[i] == accessed_index) {
            lru_position = i;
            break;
        }
    }

    if (lru_position == -1) return; // Should not happen

    // Step 3: Move accessed index to the front (Most Recently Used)
    uint32_t temp = lru_list[lru_position];
    for (int i = lru_position; i > 0; i--) {
        lru_list[i] = lru_list[i - 1];
    }
    lru_list[0] = temp;
}

//this function prefers evicting a "clean" cache line 
//if all lines are dirty, evict the least recently used cache line 
uint32_t lru_prefer_clean_eviction_index(struct replacement_policy *replacement_policy,
                                        struct cache_system *cache_system, uint32_t set_idx)
{
    struct lru_metadata *metadata = (struct lru_metadata *)replacement_policy->data;
    if (!metadata) return 0; // Prevent NULL access

    uint32_t *lru_list = metadata->lru_list[set_idx];
    
    // First pass: Try to find a clean line based on LRU order
    for (int i = cache_system->associativity - 1; i >= 0; i--) {
        // Get the index of the cache line from the LRU list
        uint32_t line_index = lru_list[i];
        
        // Check if this line is clean (not MODIFIED)
        uint32_t cache_line_idx = set_idx * cache_system->associativity + line_index;
        if (cache_system->cache_lines[cache_line_idx].status != MODIFIED) {
            return line_index; // Found a clean line to evict
        }
    }
    
    // If no clean line found, return the LRU index (least recently used)
    return lru_list[cache_system->associativity - 1];
}


void lru_prefer_clean_replacement_policy_cleanup(struct replacement_policy *replacement_policy)
{
    if (!replacement_policy || !replacement_policy->data) return;

    struct lru_metadata *metadata = (struct lru_metadata *)replacement_policy->data;

    if (metadata->lru_list) {
        for (uint32_t i = 0; i < metadata->num_sets; i++) {
            free(metadata->lru_list[i]);  //Free each set's LRU list
        }
        free(metadata->lru_list);  //Free the LRU list array
    }

    free(metadata);  //Free metadata itself

    //Do NOT free `replacement_policy` here! `cache_system_cleanup()` already does this.
}




struct replacement_policy *lru_prefer_clean_replacement_policy_new(uint32_t sets,
                                                                   uint32_t associativity)
{
    struct replacement_policy *lru_prefer_clean_rp = malloc(sizeof(struct replacement_policy));
    if (!lru_prefer_clean_rp) return NULL;  // Handle memory allocation failure

    lru_prefer_clean_rp->cache_access = &lru_prefer_clean_cache_access;
    lru_prefer_clean_rp->eviction_index = &lru_prefer_clean_eviction_index;
    lru_prefer_clean_rp->cleanup = &lru_prefer_clean_replacement_policy_cleanup;

    struct lru_metadata *metadata = calloc(1, sizeof(struct lru_metadata));
    if (!metadata) {
        free(lru_prefer_clean_rp);
        return NULL;
    }

    metadata->num_sets = sets;  //  Store number of sets for cleanup
    metadata->lru_list = malloc(sets * sizeof(uint32_t *));
    if (!metadata->lru_list) {
        free(metadata);
        free(lru_prefer_clean_rp);
        return NULL;
    }

    for (uint32_t i = 0; i < sets; i++) {
        metadata->lru_list[i] = malloc(associativity * sizeof(uint32_t));
        if (!metadata->lru_list[i]) {
            //  Free previously allocated memory before returning
            for (uint32_t j = 0; j < i; j++) {
                free(metadata->lru_list[j]);
            }
            free(metadata->lru_list);
            free(metadata);
            free(lru_prefer_clean_rp);
            return NULL;
        }

        //  Initialize LRU order properly
        for (uint32_t j = 0; j < associativity; j++) {
            metadata->lru_list[i][j] = j;
        }
    }

    lru_prefer_clean_rp->data = metadata;
    return lru_prefer_clean_rp;
}

