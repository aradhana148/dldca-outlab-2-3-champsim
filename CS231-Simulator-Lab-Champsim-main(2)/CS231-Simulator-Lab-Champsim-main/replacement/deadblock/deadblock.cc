#include <algorithm>
#include <iterator>
#include "sampler.h"
#include "cache.h"
#include "util.h"

extern DeadBlockPredictor dp_predictor;
void CACHE::initialize_replacement() {}

// find replacement victim
uint32_t CACHE::find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
  int dead=-1;
  for (uint32_t i=0;i<NUM_WAY;i++){
    if(dp_predictor.predict_to_be_dead(current_set[i].ip)){
      if(dead==-1 || (current_set[i].lru>current_set[dead].lru)){
        dead=i;
      }
    }
  }
  if(dead>-1){
    return dead;
  }
  
  return std::distance(current_set, std::max_element(current_set, std::next(current_set, NUM_WAY), lru_comparator<BLOCK, BLOCK>()));
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
  if (hit && type == WRITEBACK)
    return;

  auto begin = std::next(block.begin(), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);
  uint32_t hit_lru = std::next(begin, way)->lru;
  std::for_each(begin, end, [hit_lru](BLOCK& x) {
    if (x.lru <= hit_lru)
      x.lru++;
  });
  std::next(begin, way)->lru = 0; // promote to the MRU position
}

void CACHE::replacement_final_stats() {}
