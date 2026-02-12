unsigned short GetTLT(const char * val)
{
  std::string expr(val);

  // 1. Pre-processing: Remove whitespaces and convert to lowercase
  expr.erase(std::remove(expr.begin(), expr.end(), ' '), expr.end());
  std::transform(expr.begin(), expr.end(), expr.begin(), ::tolower);

  // Normalize AND operators to '&' and OR operators to '|'
  std::replace(expr.begin(), expr.end(), '*', '&');
  std::replace(expr.begin(), expr.end(), 'x', '&');
  std::replace(expr.begin(), expr.end(), '+', '|');

  unsigned short tlt_result = 0;

  // 2. Iterate through all 16 possible combinations of 4 input channels (2^4 = 16)
  // i represents the bit index (0 to 15) in the lookup table
  for (int i = 0; i < 16; ++i) {
    bool ch[5];             // Using ch[1] to ch[4] for intuitive mapping
    ch[1] = (i & 0x1);      // Channel 1 status (LSB)
    ch[2] = (i >> 1) & 0x1; // Channel 2 status
    ch[3] = (i >> 2) & 0x1; // Channel 3 status
    ch[4] = (i >> 3) & 0x1; // Channel 4 status (MSB)

    // 3. Logic Evaluation (Simple Parser)
    // We split the expression by OR ('|') and evaluate each AND-group
    bool final_logic_state = false;
    std::string or_delimiters = "|";

    size_t start = 0, end;
    while ((end = expr.find_first_of(or_delimiters, start)) != std::string::npos ||
           start < expr.length()) {
      // Extract a single term (e.g., "1&2&3")
      std::string term =
          expr.substr(start, (end == std::string::npos) ? std::string::npos : end - start);

      // Evaluate AND logic within the term
      bool term_state = true;
      for (char c : term) {
        if (c >= '1' && c <= '4') {
          // Logic AND: If channel is present in term, multiply its state
          term_state &= ch[c - '0'];
        }
      }

      // Logic OR: Result is true if any term is true
      final_logic_state |= term_state;

      if (end == std::string::npos) break;
      start = end + 1;
    }

    // 4. Set the i-th bit if the logic evaluates to true for this combination
    if (final_logic_state) { tlt_result |= (1 << i); }
  }

  return tlt_result;
}



void test()
{
  unsigned short tlt = GetTLT("1+2x3x4");
  cout <<Form("%X", tlt) << endl;
}