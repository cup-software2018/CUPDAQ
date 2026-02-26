#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "DAQConfig/TriggerLookupTable.hh"

unsigned short TriggerLookupTable::GetTLT(const char * val)
{
  // 1. Safety Checks
  if (!val || val[0] == '\0') return 0;
  std::string expr(val);

  // Remove spaces
  expr.erase(std::remove_if(expr.begin(), expr.end(), ::isspace), expr.end());
  if (expr.empty()) return 0;

  // 2. Parse Logic into Bitmasks (The "Compiler" Phase)
  // We store required bits for each OR-term.
  // Example: "1x2 + 4" -> stores { 0x3 (binary 0011), 0x8 (binary 1000) }
  std::vector<unsigned short> logic_masks;

  // Normalize splitters: treat '+' as logic OR separator
  std::replace(expr.begin(), expr.end(), '|', '+');

  std::stringstream ss(expr);
  std::string segment;

  while (std::getline(ss, segment, '+')) {
    if (segment.empty()) continue;

    unsigned short current_mask = 0;
    bool has_valid_channel = false;

    for (char c : segment) {
      if (c >= '1' && c <= '4') {
        // Convert char '1'..'4' to bit index 0..3
        // '1' -> bit 0 (1 << 0)
        // '2' -> bit 1 (1 << 1)
        int bit_idx = c - '1';
        current_mask |= (1 << bit_idx);
        has_valid_channel = true;
      }
    }

    // Only add valid terms (ignore garbage like "&&&")
    if (has_valid_channel) { logic_masks.push_back(current_mask); }
  }

  if (logic_masks.empty()) return 0;

  // 3. Evaluate Logic (The "Execution" Phase)
  unsigned short tlt_result = 0;

  // Iterate through all 16 combinations (0x0 to 0xF)
  for (unsigned short i = 0; i < 16; ++i) {
    bool match = false;

    // Check against pre-calculated masks
    // Logic: For a term to be true, the input (i) must have ALL bits of the mask set.
    // Formula: (input & mask) == mask
    for (unsigned short mask : logic_masks) {
      if ((i & mask) == mask) {
        match = true;
        break; // Short-circuit: Logic OR satisfied
      }
    }

    if (match) { tlt_result |= (1 << i); }
  }

  return tlt_result;
}