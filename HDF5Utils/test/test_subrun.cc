#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "HDF5Utils/H5DataReader.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "HDF5Utils/H5FADCEvent.hh"

// -----------------------------------------------------------------------------
// Write function: Splits data into multiple files with incrementing subruns
// -----------------------------------------------------------------------------
void WriteSplitFADCEvents(int n_files, int events_per_file, int max_channels, int ndp)
{
  std::cout << "========== [WRITE] Start writing split FADC events ==========\n";

  auto start_time = std::chrono::high_resolution_clock::now();

  for (int file_idx = 0; file_idx < n_files; ++file_idx) {
    // Increment subrun number for each file
    int subrun_num = 100 + file_idx;
    std::string filename = "test_fadc_events_subrun_" + std::to_string(subrun_num) + ".h5";

    // Setup DataWriter for the current split file
    H5DataWriter writer(filename.c_str());
    H5FADCEvent * wEvent = new H5FADCEvent();

    wEvent->SetNDP(ndp);
    wEvent->SetBufferCapacity(200);

    writer.SetData(wEvent);
    writer.SetSubrun(subrun_num);
    writer.Open();

    int start_event_id = file_idx * events_per_file;

    // Generate and append event data for this specific file
    for (int i = 0; i < events_per_file; ++i) {
      int global_event_id = start_event_id + i;

      EventInfo_t info;
      info.tnum = global_event_id;
      info.ttime = static_cast<std::uint64_t>(global_event_id) * 10000ULL;

      // Deterministic calculation for channel counts instead of stochastic methods
      int current_nhit = (global_event_id % max_channels) + 1;

      std::vector<FChannel_t> channels(current_nhit);
      for (int ch = 0; ch < current_nhit; ++ch) {
        channels[ch].id = ch;
        channels[ch].tbit = 1;
        channels[ch].ped = 200;
        channels[ch].time = info.ttime + ch;

        // Fill waveform with deterministic pattern based on IDs and indices
        for (int s = 0; s < ndp; ++s) {
          channels[ch].waveform[s] = static_cast<std::uint16_t>((global_event_id + ch + s) % 4096);
        }
      }

      wEvent->AppendEvent(info, channels);
    }

    writer.PrintStats();
    writer.Close();

    std::cout << "[File " << file_idx + 1 << "/" << n_files << "] Written: " << filename
              << " (Subrun: " << subrun_num << ")\n";
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end_time - start_time;

  std::cout << "--------------------------------------------------\n";
  std::cout << "Total Elapsed Time (All Files): " << std::fixed << std::setprecision(4)
            << elapsed.count() << " seconds\n\n";
}

// -----------------------------------------------------------------------------
// Read function: Chains multiple files together and reads them seamlessly
// -----------------------------------------------------------------------------
void ReadSplitFADCEvents(int n_files)
{
  std::cout << "========== [READ] Read and verify split FADC events (Chain) ==========\n";

  H5DataReader reader;
  H5FADCEvent * rEvent = new H5FADCEvent();

  reader.SetData(rEvent);

  // Add multiple files to the reader chain
  for (int file_idx = 0; file_idx < n_files; ++file_idx) {
    int subrun_num = 100 + file_idx;
    std::string filename = "test_fadc_events_subrun_" + std::to_string(subrun_num) + ".h5";

    reader.Add(filename.c_str());
    std::cout << "Added to chain: " << filename << "\n";
  }

  if (!reader.Open()) {
    std::cerr << "Failed to open chained files.\n";
    return;
  }

  // The total entries should be the sum of events across all chained files
  int total_entries = reader.GetEntries();
  std::cout << "\nTotal chained events to read: " << total_entries << "\n\n";

  // Check at an interval that intentionally crosses file boundaries
  int check_interval = 500;

  for (int i = 0; i < total_entries; ++i) {
    if (rEvent->ReadEvent(i) < 0) {
      std::cerr << "Failed to read global event " << i << "!\n";
      break;
    }

    if (i % check_interval == 0 || i == total_entries - 1) {
      EventInfo_t info = rEvent->GetEventInfo();
      FChannel_t * data = rEvent->GetData();

      std::cout << "[Global Event " << i << "] TNum: " << info.tnum
                << ", Triggered Channels (NHits): " << info.nhit << "\n";

      if (info.nhit > 0) {
        std::cout << "  -> Ch " << data[0].id << " Wave[0]: " << data[0].waveform[0] << "\n";
      }
      std::cout << "--------------------------------------------------\n";
    }
  }

  reader.Close();
  std::cout << "Chained FADC Event read test completed successfully.\n";
}

// -----------------------------------------------------------------------------
// Main execution
// -----------------------------------------------------------------------------
int main()
{
  const int n_files = 3;            // Number of split files to generate
  const int events_per_file = 1000; // Events per file (Total = 3000)
  const int max_channels = 16;
  const int ndp = 256;

  WriteSplitFADCEvents(n_files, events_per_file, max_channels, ndp);
  ReadSplitFADCEvents(n_files);

  return 0;
}