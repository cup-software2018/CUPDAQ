#include <bitset>
#include <iomanip>
#include <iostream>

#include "DAQConfig/TriggerLookupTable.hh"

/**
 * @brief Helper function to print TLT result in a readable format
 */
void printTLT(const char * expr, unsigned short result)
{
  std::cout << "Expression: \"" << (expr ? expr : "NULL") << "\"" << std::endl;
  std::cout << "Result (hex): 0x" << std::hex << std::setw(4) << std::setfill('0') << result
            << std::dec << std::endl;
  std::cout << "Result (bin): " << std::bitset<16>(result) << std::endl;
  std::cout << "Trigger fires for combinations:" << std::endl;
  for (int i = 0; i < 16; ++i) {
    if (result & (1 << i)) {
      std::cout << "  i=" << std::setw(2) << i << " (";
      std::cout << "ch1=" << ((i & 0x1) ? 1 : 0) << ", ";
      std::cout << "ch2=" << (((i >> 1) & 0x1) ? 1 : 0) << ", ";
      std::cout << "ch3=" << (((i >> 2) & 0x1) ? 1 : 0) << ", ";
      std::cout << "ch4=" << (((i >> 3) & 0x1) ? 1 : 0) << ")" << std::endl;
    }
  }
  std::cout << std::endl;
}

int main()
{
  TriggerLookupTable & tlt = TriggerLookupTable::Instance();

  std::cout << "=== Trigger Lookup Table Test Suite ===" << std::endl << std::endl;

  // Test 1: Basic AND operation
  std::cout << "--- Test 1: Basic AND operation ---" << std::endl;
  printTLT("1x2", tlt.GetTLT("1x2"));

  // Test 2: Basic OR operation
  std::cout << "--- Test 2: Basic OR operation ---" << std::endl;
  printTLT("1+2", tlt.GetTLT("1+2"));

  // Test 3: Complex combination
  std::cout << "--- Test 3: Complex combination (1x2 + 3x4) ---" << std::endl;
  printTLT("1x2 + 3x4", tlt.GetTLT("1x2 + 3x4"));

  // Test 4: Three-way AND
  std::cout << "--- Test 4: Three-way AND (1x2x3) ---" << std::endl;
  printTLT("1x2x3", tlt.GetTLT("1x2x3"));

  // Test 5: Single channel
  std::cout << "--- Test 5: Single channel (3) ---" << std::endl;
  printTLT("3", tlt.GetTLT("3"));

  // Test 6: All four channels (1x2x3x4)
  std::cout << "--- Test 6: All four channels AND ---" << std::endl;
  printTLT("1x2x3x4", tlt.GetTLT("1x2x3x4"));

  // Test 7: Mixed operators
  std::cout << "--- Test 7: Mixed operators (1*2 + 3&4) ---" << std::endl;
  printTLT("1*2 + 3&4", tlt.GetTLT("1*2 + 3&4"));

  // Test 8: Edge case - NULL input
  std::cout << "--- Test 8: NULL input ---" << std::endl;
  printTLT(nullptr, tlt.GetTLT(nullptr));

  // Test 9: Edge case - Empty string
  std::cout << "--- Test 9: Empty string ---" << std::endl;
  printTLT("", tlt.GetTLT(""));

  // Test 10: Edge case - Only whitespace
  std::cout << "--- Test 10: Only whitespace ---" << std::endl;
  printTLT("   ", tlt.GetTLT("   "));

  // Test 11: Invalid channel number (should be ignored)
  std::cout << "--- Test 11: Invalid channel number (1x5) ---" << std::endl;
  printTLT("1x5", tlt.GetTLT("1x5"));

  // Test 12: Duplicate channels (1x1)
  std::cout << "--- Test 12: Duplicate channels (1x1) ---" << std::endl;
  printTLT("1x1", tlt.GetTLT("1x1"));

  // Test 13: Empty terms (1||2)
  std::cout << "--- Test 13: Empty terms (1||2) ---" << std::endl;
  printTLT("1||2", tlt.GetTLT("1||2"));

  // Test 14: Complex real-world example
  std::cout << "--- Test 14: Complex coincidence (1x2 + 1x3 + 2x3) ---" << std::endl;
  printTLT("1x2 + 1x3 + 2x3", tlt.GetTLT("1x2 + 1x3 + 2x3"));

  // Test 15: All channels OR
  std::cout << "--- Test 15: All channels OR (1+2+3+4) ---" << std::endl;
  printTLT("1+2+3+4", tlt.GetTLT("1+2+3+4"));

  return 0;
}