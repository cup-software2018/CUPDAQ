#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include "HDF5Utils/H5DataReader.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "HDF5Utils/H5FADCHit.hh"

// -----------------------------------------------------------------------------
// Write function
// -----------------------------------------------------------------------------
void WriteFADCHit(const char * filename, int n_hits, int ndp)
{
  // Initialize number generator for simulated channel IDs and ADC values
  std::mt19937 rng(112233);
  std::uniform_int_distribution<int> dist_id(0, 63);    // 64 possible channels
  std::uniform_int_distribution<int> dist_adc(0, 4095); // 12-bit ADC

  std::cout << "========== [WRITE] Start writing FADC hits (Self Trigger) ==========\n";

  // Setup DataWriter
  H5DataWriter writer(filename);

  H5FADCHit * wHit = new H5FADCHit();
  wHit->SetNDP(ndp);
  wHit->SetBufferCapacity(5000); // Flush to file every 5000 hits (smaller individual size)

  writer.SetData(wHit);
  writer.SetSubrun(777);

  // Start the timer
  auto start_time = std::chrono::high_resolution_clock::now();

  writer.Open();

  // Generate and append individual hit data
  for (int i = 0; i < n_hits; ++i) {
    FChannel_t hit;
    hit.id = static_cast<std::uint16_t>(dist_id(rng));
    hit.tbit = 1;
    hit.ped = 200;
    hit.time = static_cast<std::uint64_t>(i) * 150ULL; // Simulated sequential time

    // Fill waveform data
    for (int s = 0; s < ndp; ++s) {
      hit.waveform[s] = static_cast<std::uint16_t>(dist_adc(rng));
    }

    // Append a single hit directly
    wHit->AppendHit(hit);
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
  // For Hit mode, there is no EventInfo_t. Just the header and the waveform array.
  double raw_header_bytes = static_cast<double>(n_hits * sizeof(FChannelHeader_t));
  double raw_wave_bytes = static_cast<double>(n_hits * ndp * sizeof(std::uint16_t));

  double uncompressed_mb = (raw_header_bytes + raw_wave_bytes) / (1024.0 * 1024.0);

  // Avoid division by zero if file creation failed
  double compression_ratio = (file_size_mb > 0.0) ? (uncompressed_mb / file_size_mb) : 0.0;

  double speed_mb_per_sec = file_size_mb / elapsed_sec;
  double speed_hits_per_sec = n_hits / elapsed_sec;

  // Print detailed write statistics
  std::cout << "\n--- Write Statistics Summary ---\n";
  std::cout << "Total Hits Written     : " << n_hits << "\n";
  std::cout << "Data Points Per Hit    : " << ndp << "\n";
  std::cout << "Total Waveform Samples : " << static_cast<long long>(n_hits) * ndp << "\n";
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
  std::cout << "Write Speed (Hits)     : " << std::fixed << std::setprecision(2)
            << speed_hits_per_sec << " Hits/s\n";
  std::cout << "--------------------------------\n\n";
}

// -----------------------------------------------------------------------------
// Read function
// -----------------------------------------------------------------------------
void ReadFADCHit(const char * filename)
{
  std::cout << "========== [READ] Read and verify FADC hits ==========\n";

  // Setup DataReader
  H5DataReader reader;
  H5FADCHit * rHit = new H5FADCHit();

  reader.Add(filename);
  reader.SetData(rHit);

  if (!reader.Open()) {
    std::cerr << "Failed to open the file.\n";
    return;
  }

  // GetEntries() now represents the total number of individual hits
  int total_entries = reader.GetEntries();
  std::cout << "Total hits to read: " << total_entries << "\n\n";

  // Read and verify data
  int check_interval = 20000; // Print and check every 20,000 hits

  for (int i = 0; i < total_entries; ++i) {
    if (rHit->ReadHit(i) < 0) {
      std::cerr << "Failed to read hit " << i << "!\n";
      break;
    }

    if (i % check_interval == 0 || i == total_entries - 1) {
      // Access the unified FChannel_t structure directly
      const FChannel_t & hit = rHit->GetHit();

      std::cout << "[Hit Index " << i << "] ID: " << hit.id << ", Time: " << hit.time
                << ", Ped: " << hit.ped << ", Wave[0]: " << hit.waveform[0] << "\n";
    }
  }

  reader.Close();
  std::cout << "--------------------------------------------------\n";
  std::cout << "FADC Hit read test completed successfully.\n";
}

// -----------------------------------------------------------------------------
// Main execution
// -----------------------------------------------------------------------------
int main()
{
  const char * filename = "test_fadc_hits.h5";

  // Since hits are individual channels (no event grouping),
  // we use a significantly larger number to generate a measurable file size.
  const int n_hits = 100000;
  const int ndp = 256;

  WriteFADCHit(filename, n_hits, ndp);
  ReadFADCHit(filename);

  return 0;
}