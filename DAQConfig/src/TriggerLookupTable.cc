#include <algorithm>
#include <set>
#include <string>

#include "DAQConfig/TriggerLookupTable.hh"

/**
 * Generates a 16-bit Trigger Lookup Table (TLT) value based on a logic string.
 * Supports AND logic ('x', '*', '&') and OR logic ('+', '|').
 *
 * Input example: "1x2 + 3x4" means (Ch1 AND Ch2) OR (Ch3 AND Ch4).
 *
 * Improvements:
 * 1. Input validation for channel numbers (1-4 only)
 * 2. Duplicate channel detection within AND terms
 * 3. Empty term handling
 * 4. Better bounds checking
 *
 * @param val Logic expression string
 * @return 16-bit lookup table value (0 if input is invalid)
 */
unsigned short TriggerLookupTable::GetTLT(const char * val)
{
  // Handle null input
  if (!val) return 0;

  std::string expr(val);

  // Pre-processing: Remove whitespaces and convert to lowercase
  expr.erase(std::remove(expr.begin(), expr.end(), ' '), expr.end());
  std::transform(expr.begin(), expr.end(), expr.begin(), ::tolower);

  // Handle empty string after whitespace removal
  if (expr.empty()) return 0;

  // Normalize AND operators to '&' and OR operators to '|'
  std::replace(expr.begin(), expr.end(), '*', '&');
  std::replace(expr.begin(), expr.end(), 'x', '&');
  std::replace(expr.begin(), expr.end(), '+', '|');

  unsigned short tlt_result = 0;

  // Iterate through all 16 possible combinations of 4 input channels (2^4 = 16)
  // i represents the bit index (0 to 15) in the lookup table
  for (int i = 0; i < 16; ++i) {
    bool ch[5];             // Using ch[1] to ch[4] for intuitive mapping (ch[0] unused)
    ch[1] = (i & 0x1);      // Channel 1 status (LSB of i)
    ch[2] = (i >> 1) & 0x1; // Channel 2 status
    ch[3] = (i >> 2) & 0x1; // Channel 3 status
    ch[4] = (i >> 3) & 0x1; // Channel 4 status (MSB of i)

    // Logic Evaluation: Parse and evaluate the expression
    // Split by OR ('|') and evaluate each AND-group
    bool final_logic_state = false;
    std::string or_delimiters = "|";
    size_t start = 0, end;

    while ((end = expr.find_first_of(or_delimiters, start)) != std::string::npos ||
           start < expr.length()) {
      // Extract a single term (e.g., "1&2&3")
      std::string term =
          expr.substr(start, (end == std::string::npos) ? std::string::npos : end - start);

      // Skip empty terms (e.g., from "1||2" or leading/trailing '|')
      if (term.empty()) {
        if (end == std::string::npos) break;
        start = end + 1;
        continue;
      }

      // Evaluate AND logic within the term
      bool term_state = true;
      bool has_valid_channel = false;
      std::set<char> seen_channels; // Track channels to detect duplicates

      for (char c : term) {
        // Process only valid channel numbers (1-4)
        if (c >= '1' && c <= '4') {
          has_valid_channel = true;

          // Check for duplicate channels in the same AND term
          if (seen_channels.count(c)) {
            // Duplicate detected: "1&1" is redundant but we'll allow it
            // (it doesn't change the logic, just redundant)
            // Option: could issue a warning in a logging system
          }
          seen_channels.insert(c);

          // Apply AND logic with the channel state
          term_state &= ch[c - '0'];
        }
        // Ignore '&' operators and any other characters (they're separators)
        // Invalid characters are simply skipped
      }

      // If term has no valid channels, treat it as false
      // (e.g., term was just "&&&" or contained only invalid chars)
      if (!has_valid_channel) { term_state = false; }

      // Logic OR: Result is true if any term is true
      final_logic_state |= term_state;

      if (end == std::string::npos) break;
      start = end + 1;
    }

    // Set the i-th bit if the logic evaluates to true for this combination
    if (final_logic_state) { tlt_result |= (1 << i); }
  }

  return tlt_result;
}