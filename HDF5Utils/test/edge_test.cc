#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "HDF5Utils/H5DataReader.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "HDF5Utils/H5FADCEvent.hh"

// -----------------------------------------------------------------------------
// [Test 1] Empty Run (Zero Events)
// Verifies if the system can safely open and close a file without any data.
// -----------------------------------------------------------------------------
void TestEmptyRun()
{
  std::cout << "========== [Test 1] Empty Run (Zero Events) ==========\n";
  const char * filename = "test_edge_empty.h5";

  // Write Phase
  H5DataWriter writer(filename);
  H5FADCEvent * wEvent = new H5FADCEvent();
  wEvent->SetNDP(100);
  writer.SetData(wEvent);
  writer.Open();

  // Immediately close without calling AppendEvent
  writer.Close();
  delete wEvent;

  std::cout << "Empty file created successfully.\n";

  // Read Phase
  H5DataReader reader;
  H5FADCEvent * rEvent = new H5FADCEvent();
  reader.Add(filename);
  reader.SetData(rEvent);

  if (reader.Open()) {
    int entries = reader.GetEntries();
    std::cout << "Total entries in empty file: " << entries << " (Expected: 0)\n";
    if (entries == 0) { std::cout << "[PASS] Test 1: Empty Run handled safely.\n"; }
    else {
      std::cout << "[FAIL] Test 1: Entries should be 0.\n";
    }
  }
  else {
    std::cout << "[FAIL] Test 1: Failed to open empty file.\n";
  }

  reader.Close();
  delete rEvent;
  std::cout << "------------------------------------------------------\n\n";
}

// -----------------------------------------------------------------------------
// [Test 2] HDF5 Handle Leak Check
// Verifies if repeated rapid open/close cycles cause resource exhaustion.
// -----------------------------------------------------------------------------
void TestHandleLeak()
{
  std::cout << "========== [Test 2] HDF5 Handle Leak Check ==========\n";
  const char * filename = "test_edge_leak.h5";
  const int loop_count = 100;

  std::cout << "Performing " << loop_count << " rapid open/write/close cycles...\n";

  bool success = true;
  for (int i = 0; i < loop_count; ++i) {
    H5DataWriter writer(filename);
    H5FADCEvent * wEvent = new H5FADCEvent();
    wEvent->SetNDP(10);
    writer.SetData(wEvent);

    if (!writer.Open()) {
      std::cout << "Failed to open file at iteration " << i << "!\n";
      success = false;
      delete wEvent;
      break;
    }

    EventInfo_t info;
    info.tnum = i;
    info.ttime = i * 100ULL;
    info.nhit = 0;

    std::vector<FChannel_t> channels; // Empty channel vector
    wEvent->AppendEvent(info, channels);

    writer.Close();
    delete wEvent;
  }

  if (success) {
    std::cout << "[PASS] Test 2: Survived all cycles without HDF5 handle exhaustion.\n";
  }
  else {
    std::cout << "[FAIL] Test 2: HDF5 handle leak detected.\n";
  }
  std::cout << "------------------------------------------------------\n\n";
}

// -----------------------------------------------------------------------------
// [Test 3] Buffer Boundary and Residual Data
// Verifies if remaining events in the buffer are correctly flushed upon Close().
// -----------------------------------------------------------------------------
void TestBufferBoundary()
{
  std::cout << "========== [Test 3] Buffer Boundary & Residual Data ==========\n";
  const char * filename = "test_edge_boundary.h5";

  const int buffer_capacity = 100;
  const int target_events = 105; // 100 flushed + 5 residual events

  // Write Phase
  H5DataWriter writer(filename);
  H5FADCEvent * wEvent = new H5FADCEvent();
  wEvent->SetNDP(10);
  wEvent->SetBufferCapacity(buffer_capacity);
  writer.SetData(wEvent);
  writer.Open();

  for (int i = 0; i < target_events; ++i) {
    EventInfo_t info;
    info.tnum = i;
    info.ttime = i * 10ULL;

    std::vector<FChannel_t> channels(1);
    channels[0].id = 0;
    channels[0].time = i;
    for (int s = 0; s < 10; ++s) {
      channels[0].waveform[s] = static_cast<std::uint16_t>(s);
    }
    wEvent->AppendEvent(info, channels);
  }
  writer.Close(); // This should trigger the final flush for the 5 residual events
  delete wEvent;

  // Read Phase
  H5DataReader reader;
  H5FADCEvent * rEvent = new H5FADCEvent();
  reader.Add(filename);
  reader.SetData(rEvent);

  if (reader.Open()) {
    int read_entries = reader.GetEntries();
    std::cout << "Expected entries: " << target_events << ", Read entries: " << read_entries
              << "\n";

    if (read_entries == target_events) {
      rEvent->ReadEvent(target_events - 1);
      std::uint32_t last_tnum = rEvent->GetEventInfo().tnum;
      std::cout << "Last event TNum: " << last_tnum << " (Expected: " << target_events - 1 << ")\n";

      if (last_tnum == static_cast<std::uint32_t>(target_events - 1)) {
        std::cout << "[PASS] Test 3: Residual data flushed correctly.\n";
      }
      else {
        std::cout << "[FAIL] Test 3: Last event mismatch.\n";
      }
    }
    else {
      std::cout << "[FAIL] Test 3: Data loss occurred at buffer boundary.\n";
    }
  }

  reader.Close();
  delete rEvent;
  std::cout << "------------------------------------------------------\n\n";
}

// -----------------------------------------------------------------------------
// [Test 4] Broken Chain / Corrupted File Handling
// Verifies if the reader can gracefully handle a broken/non-HDF5 file in a chain.
// -----------------------------------------------------------------------------
void TestBrokenChain()
{
  std::cout << "========== [Test 4] Broken Chain / Corrupted File ==========\n";

  const char * valid_file1 = "test_edge_valid1.h5";
  const char * corrupted_file = "test_edge_corrupted.h5";
  const char * valid_file2 = "test_edge_valid2.h5";

  // Create Valid File 1
  H5DataWriter w1(valid_file1);
  H5FADCEvent * e1 = new H5FADCEvent();
  e1->SetNDP(10);
  w1.SetData(e1);
  w1.Open();
  EventInfo_t info;
  info.tnum = 1;
  info.ttime = 1;
  std::vector<FChannel_t> ch;
  e1->AppendEvent(info, ch);
  w1.Close();
  delete e1;

  // Create Corrupted File (Fake text file disguised as .h5)
  std::ofstream out(corrupted_file);
  out << "This is completely garbage text data, not an HDF5 binary format.";
  out.close();

  // Create Valid File 2
  H5DataWriter w2(valid_file2);
  H5FADCEvent * e2 = new H5FADCEvent();
  e2->SetNDP(10);
  w2.SetData(e2);
  w2.Open();
  e2->AppendEvent(info, ch);
  w2.Close();
  delete e2;

  // Attempt to read the chain
  std::cout << "Attempting to load chain: Valid -> Corrupted -> Valid\n";

  H5DataReader reader;
  H5FADCEvent * rEvent = new H5FADCEvent();
  reader.SetData(rEvent);

  reader.Add(valid_file1);
  reader.Add(corrupted_file);
  reader.Add(valid_file2);

  // Suppress default HDF5 error printing just for this test (optional, but cleaner)
  // H5Eset_auto2(H5E_DEFAULT, NULL, NULL);

  bool isOpen = reader.Open();
  std::cout << "Reader Open() returned: " << (isOpen ? "true" : "false") << "\n";

  if (!isOpen) {
    std::cout << "[PASS] Test 4: Gracefully rejected corrupted chain without crashing.\n";
  }
  else {
    // If it opened, check if GetEntries() behaves safely
    int entries = reader.GetEntries();
    std::cout << "Total entries reported: " << entries << "\n";
    std::cout << "[WARNING] Test 4: Reader forced open, ensure ReadEvent handles errors.\n";
  }

  reader.Close();
  delete rEvent;
  std::cout << "------------------------------------------------------\n\n";
}

// -----------------------------------------------------------------------------
// Main execution
// -----------------------------------------------------------------------------
int main()
{
  std::cout << "Starting Robustness Edge Case Tests...\n\n";

  TestEmptyRun();
  TestHandleLeak();
  TestBufferBoundary();
  TestBrokenChain();

  std::cout << "All Robustness Edge Case Tests Finished.\n";
  return 0;
}