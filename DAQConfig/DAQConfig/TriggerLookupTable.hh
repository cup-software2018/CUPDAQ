
#ifndef TriggerLookupTable_hh
#define TriggerLookupTable_hh

/**
 * @class TriggerLookupTable
 * @brief Generates 16-bit trigger lookup tables from logic expressions
 *
 * This singleton class converts trigger logic expressions (e.g., "1x2 + 3x4")
 * into 16-bit lookup tables where each bit represents whether the trigger
 * should fire for a given combination of 4 input channels.
 *
 * Supported operators:
 * - AND: 'x', '*', '&'
 * - OR: '+', '|'
 *
 * Valid channel numbers: 1-4
 */
class TriggerLookupTable {
public:
  static TriggerLookupTable & Instance()
  {
    static TriggerLookupTable instance;
    return instance;
  }

  /**
   * @brief Generate a 16-bit trigger lookup table from a logic expression
   * @param val Logic expression string (e.g., "1x2 + 3x4")
   * @return 16-bit value where bit i indicates trigger state for channel combination i
   * @throws None - returns 0 for invalid input
   */
  unsigned short GetTLT(const char * val);

private:
  TriggerLookupTable() = default;
  ~TriggerLookupTable() = default;

  // Prevent copying
  TriggerLookupTable(const TriggerLookupTable &) = delete;
  TriggerLookupTable & operator=(const TriggerLookupTable &) = delete;
};

#endif