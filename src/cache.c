//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.h"

//
// TODO:Student Information
//
const char *studentName = "Tianyi Wang";
const char *studentID   = "A53274015";
const char *email       = "t9wang@ucsd.edu";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;     // Number of sets in the I$
uint32_t icacheAssoc;    // Associativity of the I$
uint32_t icacheHitTime;  // Hit Time of the I$

uint32_t dcacheSets;     // Number of sets in the D$
uint32_t dcacheAssoc;    // Associativity of the D$
uint32_t dcacheHitTime;  // Hit Time of the D$

uint32_t l2cacheSets;    // Number of sets in the L2$
uint32_t l2cacheAssoc;   // Associativity of the L2$
uint32_t l2cacheHitTime; // Hit Time of the L2$
uint32_t inclusive;      // Indicates if the L2 is inclusive

uint32_t blocksize;      // Block/Line size
uint32_t memspeed;       // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
//
//icache ds
uint32_t*** icache;
uint32_t** icacheset;
uint32_t* isetway;
uint32_t ioffset;
uint32_t iindex;
uint32_t itag;
//dcache ds
uint32_t*** dcache;
uint32_t** dcacheset;
uint32_t* dsetway;
uint32_t doffset;
uint32_t dindex;
uint32_t dtag;
//l2cache ds
uint32_t*** l2cache;
uint32_t** l2cacheset_ds;
uint32_t* l2setway_ds;
uint32_t l2offset;
uint32_t l2index;
uint32_t l2tag;
//flags
_Bool i_hit;
_Bool d_hit;
_Bool l2_hit;
_Bool i_miss;
_Bool d_miss;
_Bool l2_miss;
//inclussive ds
uint32_t inclusive_add;
uint8_t inclusive_src;

//------------------------------------//
//          Cache Functions           //
//------------------------------------//

// Initialize the Cache Hierarchy
//
void
init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;
  
  //
  //TODO: Initialize Cache Simulator Data Structures
  //
  icache = malloc(icacheSets * sizeof(uint32_t**));
  dcache = malloc(dcacheSets * sizeof(uint32_t**));
  l2cache = malloc(l2cacheSets * sizeof(uint32_t**));
  int isize = 3;
  int dsize = 3;
  int l2size = 4;

  for(int i = 0; i < icacheSets; i++)
  {
    icache[i] = malloc(icacheAssoc * sizeof(uint32_t*));
    for(int j = 0; j < icacheAssoc; j++)
    {
      icache[i][j] = malloc(isize * sizeof(uint32_t)); 
      icache[i][j][1] = 0; 
      icache[i][j][2] = j;
    }
  }

  for(int i = 0; i < dcacheSets; i++)
  {
    dcache[i] = malloc(dcacheAssoc * sizeof(uint32_t*));
    
    for(int j = 0; j < dcacheAssoc; j++)
    {
      dcache[i][j] = malloc(dsize * sizeof(uint32_t)); 
      dcache[i][j][1] = 0;
      dcache[i][j][2] = j;
    }
  }

  for(int i = 0; i < l2cacheSets; i++)
  {
    l2cache[i] = malloc(l2cacheAssoc * sizeof(uint32_t*));
    
    for(int j = 0; j < l2cacheAssoc; j++)
    {
      l2cache[i][j] = malloc(l2size * sizeof(uint32_t)); 
      l2cache[i][j][1] = 0;
      l2cache[i][j][2] = j;
      l2cache[i][j][3] = 0;
    }
  }
}

//======================================LOG===================================
// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
//function to calculate the log of the input
uint32_t log_function(uint32_t size)
{
  int power = 0;
  while(size > 1)
  {
    power += 1;
    size /= 2;
  }
  return power;
}
//=============================================================================

//=============================================================================
//===================================I=========================================
//=============================================================================
uint32_t
icache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //
  icacheRefs += 1;
  i_hit = 0;
  i_miss = 0;
  uint32_t i_addr = addr;

  if (icacheSets == 0)  return l2cache_access(addr);

  uint32_t i_total_bits = 32;
  uint32_t i_offset_bits = log_function(blocksize);
  uint32_t i_index_bits = log_function(icacheSets);
  uint32_t i_tag_bits = i_total_bits - i_offset_bits - i_index_bits;

  ioffset = i_addr & ((1 << i_offset_bits) - 1);
  i_addr >>= i_offset_bits;
  iindex = i_addr & ((1 << i_index_bits) - 1);
  i_addr >>= i_index_bits;
  itag = i_addr & ((1 << i_tag_bits) - 1);
  i_addr >>= i_tag_bits;

  for(int i = 0; i < icacheAssoc; i++)
  {
    if(icache[iindex][i][1] == 1)
    {
      if(icache[iindex][i][0] == itag)
      {
        i_hit = 1;
        i_miss = 0;
        for(int j = 0; j < icacheAssoc; j++)
        {
          if(icache[iindex][j][2] > icache[iindex][i][2]) icache[iindex][j][2] -= 1;
        }
      }
    }
    icache[iindex][i][2] = icacheAssoc - 1;
  }

  //if cache hits! return the hit time
  if(i_hit == 1) return icacheHitTime;
  //if not hit, then try l2cache
  else
  {
    i_hit = 0;
    i_miss = 1;
    icacheMisses += 1;
    int l2_memspeed = l2cache_access(addr);

    //if l2cache hits! 
    if(l2_hit == 1)
    {
      int i_flag = 0;
      for(int i = 0; i < icacheAssoc; i++)
      {
        //when data is not valid here
        if(icache[iindex][i][1] == 0)
        {
          i_flag = 1;
          icache[iindex][i][0] = itag;
          icache[iindex][i][1] = 1;
          //not here
          for(int j = 0; j < icacheAssoc; j++)
          {
            if(icache[iindex][j][2] > icache[iindex][i][2]) icache[iindex][j][2] -= 1;
          }
          icache[iindex][i][2] = icacheAssoc - 1;
        }
      }

      if(i_flag == 0)
      {
        for(int i = 0; i < icacheAssoc; i++)
        {
          if(icache[iindex][2] == 0)
          {
            icache[iindex][i][0] = itag;
            icache[iindex][i][1] = 1;
            icache[iindex][i][2] = icacheAssoc - 1;
          }
          else icache[iindex][i][2] -= 1;
        }
      }
    }
    //if l2cache does not hit.
    else
    {
      //if inclusive from icache
      if((inclusive) && (inclusive_src == 9))
      {
        int tem_index;
        int tem_tag;
        uint32_t iindex_mask = (1 << i_index_bits) -1;
        uint32_t itag_mask = (1 << i_tag_bits) - 1;
        tem_index = (inclusive_add >> i_offset_bits)& iindex_mask;
        tem_tag = ((inclusive_add >> i_offset_bits) >> i_index_bits) & itag_mask;
        for(int i = 0; i < icacheAssoc; i++)
        {
          //if found in the cache, do invalidation
          if(icache[tem_index][i][0] == tem_tag)
          {
            icache[tem_index][i][1] = 0;
            break;
          }
        }
      }
      //if inclusive from dcache
      if((inclusive) && (inclusive_src == 4))
      {
        //same as above
        uint32_t d_total_bits = 32;
        uint32_t d_offset_bits = log_function(blocksize);
        uint32_t d_index_bits = log_function(dcacheSets);
        uint32_t d_tag_bits = d_total_bits - d_offset_bits - d_index_bits;

        int tem_index;
        int tem_tag;
        uint32_t dindex_mask = (1 << d_index_bits) -1;
        uint32_t dtag_mask = (1 << d_tag_bits) - 1;
        tem_index = (inclusive_add >> d_offset_bits)& dindex_mask;
        tem_tag = ((inclusive_add >> d_offset_bits) >> d_index_bits) & dtag_mask;

        for(int i = 0; i < dcacheAssoc; i++)
        {
          //if found in the cache, do invalidation
          if(dcache[tem_index][i][0] == tem_tag)
          {
            dcache[tem_index][i][1] = 0;
            break;
          }
        }
      }
      //same as above
      int i_flag = 0;

      for(int i = 0; i < icacheAssoc; i++)
      {
        if(icache[iindex][i][1] == 0)
        {
          i_flag = 1;
          icache[iindex][i][0] = itag;
          icache[iindex][i][1] = 1;
          icache[iindex][i][2] = icacheAssoc - 1;
          for(int j = 0; j < icacheAssoc; j++)
          {
            if(icache[iindex][j][2] > icache[iindex][i][2]) icache[iindex][j][2] -= 1;
          }
        }
      }

      if(i_flag == 0)
      {
        for(int i = 0; i < icacheAssoc; i++)
        {
          if(icache[iindex][i][2] == 0)
          {
            icache[iindex][i][0] = itag;
            icache[iindex][i][1] = 1;
            icache[iindex][i][2] = icacheAssoc - 1;
          }
          else icache[iindex][i][2] -= 1;
        }
      }
    }
    icachePenalties += l2_memspeed;
    uint32_t mem = icacheHitTime + l2_memspeed;
    return mem;
  }

  return memspeed;
}

//=============================================================================
//===================================D=========================================
//=============================================================================
// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  //
  //TODO: Implement D$
  //
  dcacheRefs += 1;
  d_hit = 0;
  d_miss = 0;
  uint32_t d_addr = addr;
  
  if (dcacheSets == 0)  return l2cache_access(d_addr);

  //same as in icache
  uint32_t d_total_bits = 32;
  uint32_t d_offset_bits = log_function(blocksize);
  uint32_t d_index_bits = log_function(dcacheSets);
  uint32_t d_tag_bits = d_total_bits - d_offset_bits - d_index_bits;

  doffset = d_addr & ((1 << d_offset_bits) - 1);
  d_addr >>= d_offset_bits;
  dindex = d_addr & ((1 << d_index_bits) - 1);
  d_addr >>= d_index_bits;
  dtag = d_addr & ((1 << d_tag_bits) - 1);
  d_addr >>= d_tag_bits;

  for(int i = 0; i < dcacheAssoc; i++)
  {
    if(dcache[dindex][i][1] == 1)
    {
      if(dcache[dindex][i][0] == dtag)
      {
        d_hit = 1;
        d_miss = 0;
        for(int j = 0; j < dcacheAssoc; j++)
        {
          if(dcache[dindex][j][2] > dcache[dindex][i][2]) dcache[dindex][j][2] -= 1;
        }
      }
    }
    dcache[dindex][i][2] = dcacheAssoc - 1;
  }

  //if cache hits! return the hit time
  if(d_hit == 1) return dcacheHitTime;
  //if not hit, then try l2cache
  else
  {
    d_hit = 0;
    d_miss = 1;
    dcacheMisses += 1;
    int l2_memspeed = l2cache_access(addr);

    //if l2cache hits! 
    if(l2_hit == 1)
    {
      int d_flag = 0;
      for(int i = 0; i < dcacheAssoc; i++)
      {
        //when data is not valid here
        if(dcache[dindex][i][1] == 0)
        {
          d_flag = 1;
          dcache[dindex][i][0] = dtag;
          dcache[dindex][i][1] = 1;
          //no
          for(int j = 0; j < dcacheAssoc; j++)
          {
            if(dcache[dindex][j][2] > dcache[dindex][i][2]) dcache[dindex][j][2] -= 1;
          }
          dcache[dindex][i][2] = dcacheAssoc - 1;
        }
      }

      if(d_flag == 0)
      {
        for(int i = 0; i < dcacheAssoc; i++)
        {
          if(dcache[dindex][2] == 0)
          {
            dcache[dindex][i][0] = dtag;
            dcache[dindex][i][1] = 1;
            dcache[dindex][i][2] = dcacheAssoc - 1;
          }
          else dcache[dindex][i][2] -= 1;
        }
      }
    }
    //if l2cache does not hit.
    else
    {
      //if inclusive from icache
      if((inclusive) && (inclusive_src == 9))
      {
        uint32_t i_total_bits = 32;
        uint32_t i_offset_bits = log_function(blocksize);
        uint32_t i_index_bits = log_function(icacheSets);
        uint32_t i_tag_bits = i_total_bits - i_offset_bits - i_index_bits;

        int tem_index;
        int tem_tag;
        uint32_t iindex_mask = (1 << i_index_bits) -1;
        uint32_t itag_mask = (1 << i_tag_bits) - 1;
        tem_index = (inclusive_add >> i_offset_bits) & iindex_mask;
        tem_tag = ((inclusive_add >> i_offset_bits) >> i_index_bits) & itag_mask;
        for(int i = 0; i < icacheAssoc; i++)
        {
          //if found in the cache, do invalidation
          if(icache[tem_index][i][0] == tem_tag)
          {
            icache[tem_index][i][1] = 0;
            break;
          }
        }
      }
      //if inclusive from dcache
      if((inclusive) && (inclusive_src == 4))
      {
        //same as above
        int tem_index;
        int tem_tag;
        uint32_t dindex_mask = (1 << d_index_bits) -1;
        uint32_t dtag_mask = (1 << d_tag_bits) - 1;
        tem_index = (inclusive_add >> d_offset_bits)& dindex_mask;
        tem_tag = ((inclusive_add >> d_offset_bits) >> d_index_bits) & dtag_mask;

        for(int i = 0; i < dcacheAssoc; i++)
        {
          //if found in the cache, do invalidation
          if(dcache[tem_index][i][0] == tem_tag)
          {
            dcache[tem_index][i][1] = 0;
            break;
          }
        }
      }
      //same as in icache
      int d_flag = 0;

      for(int i = 0; i < dcacheAssoc; i++)
      {
        if(dcache[dindex][i][1] == 0)
        {
          d_flag = 1;
          dcache[dindex][i][0] = dtag;
          dcache[dindex][i][1] = 1;
          //not here
          for(int j = 0; j < dcacheAssoc; j++)
          {
            if(dcache[dindex][j][2] > dcache[dindex][i][2]) dcache[dindex][j][2] -= 1;
          }
          dcache[dindex][i][2] = dcacheAssoc - 1;
        }
      }

      if(d_flag == 0)
      {
        for(int i = 0; i < dcacheAssoc; i++)
        {
          if(dcache[dindex][i][2] == 0)
          {
            dcache[dindex][i][0] = dtag;
            dcache[dindex][i][1] = 1;
            dcache[dindex][i][2] = dcacheAssoc - 1;
          }
          else dcache[dindex][i][2] -= 1;
        }
      }
    }
    dcachePenalties += l2_memspeed;
    uint32_t mem = dcacheHitTime + l2_memspeed;
    return mem;
  }

  return memspeed;
}

//=============================================================================
//===================================L2========================================
//=============================================================================
// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
l2cache_access(uint32_t addr)
{
  //
  //TODO: Implement L2$
  //
  l2cacheRefs += 1;
  l2_hit = 0;
  l2_miss = 0;
  uint32_t l2_addr = addr;

  if (icacheSets == 0)  return l2cache_access(addr);

  uint32_t l2_total_bits = 32;
  uint32_t l2_offset_bits = log_function(blocksize);
  uint32_t l2_index_bits = log_function(l2cacheSets);
  uint32_t l2_tag_bits = l2_total_bits - l2_offset_bits - l2_index_bits;

  l2offset = l2_addr & ((1 << l2_offset_bits) - 1);
  l2_addr >>= l2_offset_bits;
  l2index = l2_addr & ((1 << l2_index_bits) - 1);
  l2_addr >>= l2_index_bits;
  l2tag = l2_addr & ((1 << l2_tag_bits) - 1);
  l2_addr >>= l2_tag_bits;

  
  for(int i = 0; i < l2cacheAssoc; i++)
  {
    if(l2cache[l2index][i][1] == 1)
    {
      if(l2cache[l2index][i][0] == l2tag)
      {
        l2_hit = 1;
        l2_miss = 0;
        for(int j = 0; j < icacheAssoc; j++)
        {
          if(l2cache[l2index][j][2] > l2cache[l2index][i][2]) l2cache[l2index][j][2] -= 1;
        }
      }
    }
    l2cache[l2index][i][2] = l2cacheAssoc - 1;
  }

  //if cache hits! return the hit time
  if(l2_hit == 1) return l2cacheHitTime;
  //if not hit
  else
  {
    l2_hit = 0;
    l2_miss = 1;
    l2cacheMisses += 1;
    
    int l2_flag = 0;

    for(int i = 0; i < l2cacheAssoc; i++)
    {
      if(l2cache[l2index][i][1] == 0)
      {
        l2_flag = 1;
        l2cache[l2index][i][0] = l2tag;
        l2cache[l2index][i][1] = 1;
        l2cache[l2index][i][2] = l2cacheAssoc - 1;
        for(int j = 0; j < l2cacheAssoc; j++)
        {
          if(l2cache[l2index][j][2] > l2cache[l2index][i][2]) l2cache[l2index][j][2] -= 1;
        }
      }


      if(i_miss) l2cache[l2index][i][3] = 1;
      else if(d_miss) l2cache[l2index][i][3] = 2;

      break;
    }

    if(l2_flag == 0)
    {
      for(int i = 0; i < l2cacheAssoc; i++)
      {
        if(l2cache[l2index][i][2] == 0)
        {
          if(inclusive == 1)
          {
            inclusive_add = ((l2cache[l2index][i][0] << l2_index_bits) + l2index) << l2_offset_bits;
            inclusive_src = l2cache[l2index][i][3];
          }

          l2cache[l2index][i][0] = l2tag;
          l2cache[l2index][i][1] = 1;
          l2cache[l2index][i][2] = l2cacheAssoc - 1;
          if(i_miss) l2cache[l2index][i][3] = 9;
          else if(d_miss) l2cache[l2index][i][3] = 4;
        }
        else l2cache[l2index][i][2] -= 1;
      }     
    }
    l2cachePenalties += memspeed;
    uint32_t mem = l2cacheHitTime + memspeed;
    return mem;
  }
  return memspeed;
}
//==================================================================================================
