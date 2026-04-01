#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include "HDF5Utils/H5DataReader.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "HDF5Utils/H5SADCEvent.hh"

// -----------------------------------------------------------------------------
// Write function
// -----------------------------------------------------------------------------
void WriteSADCEvent(const char * filename, int n_events, int max_channels)
{
  // Initialize number generator for variable channels and ADC values
  std::mt19937 rng(54321); // Fixed seed for reproducibility
  std::uniform_int_distribution<int> dist_nhit(1, max_channels);
  std::uniform_int_distribution<int> dist_adc(0, 16383); // e.g., 14-bit ADC

  std::cout << "========== [WRITE] Start writing SADC events ==========\n";

  // Setup DataWriter
  H5DataWriter writer(filename);

  H5SADCEvent * wEvent = new H5SADCEvent();
  wEvent->SetBufferCapacity(1000); // Flush to file every 1000 events

  writer.SetData(wEvent);
  writer.SetSubrun(888);

  // Start the timer
  auto start_time = std::chrono::high_resolution_clock::now();

  writer.Open();

  // Generate and append event data
  int total_channels_written = 0;

  for (int i = 0; i < n_events; ++i) {
    EventInfo_t info;
    info.tnum = i;
    info.ttime = static_cast<std::uint64_t>(i) * 5000ULL;

    // Determine the number of triggered channels for the current event
    int current_nhit = dist_nhit(rng);
    total_channels_written += current_nhit;

    std::vector<AChannel_t> channels(current_nhit);
    for (int ch = 0; ch < current_nhit; ++ch) {
      channels[ch].id = ch;
      channels[ch].adc = static_cast<std::uint32_t>(dist_adc(rng));
      channels[ch].time = info.ttime + ch;
    }

    wEvent->AppendEvent(info, channels);
  }

  // Get file size before closing the file
  double file_size_mb = static_cast<double>(writer.GetFileSize()) / (1024.0 * 1024.0);

  writer.PrintStats();
  writer.Close();

  // Stop the timer
  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end_time - start_time;
  double elapsed_sec = elapsed.count();

  // ---------------------------------------------------------------------------
  // Calculate Statistics (Speed and Compression)
  // ---------------------------------------------------------------------------

  // Calculate raw uncompressed bytes based on EDM struct sizes
  // SADC does not have waveforms, only EventInfo_t and AChannel_t
  double raw_event_bytes = static_cast<double>(n_events * sizeof(EventInfo_t));
  double raw_channel_bytes = static_cast<double>(total_channels_written * sizeof(AChannel_t));

  double uncompressed_mb = (raw_event_bytes + raw_channel_bytes) / (1024.0 * 1024.0);

  // Avoid division by zero if file creation failed
  double compression_ratio = (file_size_mb > 0.0) ? (uncompressed_mb / file_size_mb) : 0.0;

  double speed_mb_per_sec = file_size_mb / elapsed_sec;
  double speed_events_per_sec = n_events / elapsed_sec;

  // Print detailed write statistics
  std::cout << "\n--- Write Statistics Summary ---\n";
  std::cout << "Total Events Written   : " << n_events << "\n";
  std::cout << "Total Channels Written : " << total_channels_written << "\n";
  std::cout << "--------------------------------\n";
  std::cout << "Uncompressed Data Size : " << std::fixed << std::setprecision(2) << uncompressed_mb
            << " MB\n";
  std::cout << "Compressed File Size   : " << std::fixed << std::setprecision(2) << file_size_mb
            << " MB\n";
  std::cout << "Compression Ratio      : " << std::fixed << std::setprecision(2)
            << compression_ratio << "x\n";
  std::cout << "--------------------------------\n";
  std::cout << "Elapsed Time           : " << std::fixed << std::setprecision(4) << elapsed_sec
            << " seconds\n";
  std::cout << "Write Speed (Disk IO)  : " << std::fixed << std::setprecision(2) << speed_mb_per_sec
            << " MB/s\n";
  std::cout << "Write Speed (Events)   : " << std::fixed << std::setprecision(2)
            << speed_events_per_sec << " Events/s\n";
  std::cout << "--------------------------------\n\n";
}

// -----------------------------------------------------------------------------
// Read function
// -----------------------------------------------------------------------------
void ReadSADCEvent(const char * filename)
{
  std::cout << "========== [READ] Read and verify SADC events ==========\n";

  // Setup DataReader
  H5DataReader reader;
  H5SADCEvent * rEvent = new H5SADCEvent();

  reader.Add(filename);
  reader.SetData(rEvent);

  if (!reader.Open()) {
    std::cerr << "Failed to open the file.\n";
    return;
  }

  int total_entries = reader.GetEntries();
  std::cout << "Total events to read: " << total_entries << "\n\n";

  // Read and verify data
  int check_interval = 5000; // Print and check every 5000 events

  for (int i = 0; i < total_entries; ++i) {
    if (rEvent->ReadEvent(i) < 0) {
      std::cerr << "Failed to read event " << i << "!\n";
      break;
    }

    if (i % check_interval == 0 || i == total_entries - 1) {
      EventInfo_t info = rEvent->GetEventInfo();
      AChannel_t * data = rEvent->GetData();

      std::cout << "[Event " << i << "] TNum: " << info.tnum
                << ", Triggered Channels (NHits): " << info.nhit << "\n";

      if (info.nhit > 0) {
        std::cout << "  -> Ch " << data[0].id << " ADC: " << data[0].adc << "\n";
        if (info.nhit > 1) {
          int last_idx = info.nhit - 1;
          std::cout << "  -> Ch " << data[last_idx].id << " ADC: " << data[last_idx].adc << "\n";
        }
      }
      std::cout << "--------------------------------------------------\n";
    }
  }

  reader.Close();
  std::cout << "SADC Event read test completed successfully.\n";
}

// -----------------------------------------------------------------------------
// Main execution
// -----------------------------------------------------------------------------
int main()
{
  const char * filename = "test_sadc_events.h5";

  // SADC data is much smaller than FADC, so use a larger number of events
  // to properly evaluate I/O speeds and compression.
  const int n_events = 50000;
  const int max_channels = 32;

  WriteSADCEvent(filename, n_events, max_channels);
  ReadSADCEvent(filename);

  return 0;
}