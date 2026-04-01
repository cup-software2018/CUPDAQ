#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include "HDF5Utils/H5DataReader.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "HDF5Utils/H5SADCHit.hh"

// -----------------------------------------------------------------------------
// Write function
// -----------------------------------------------------------------------------
void WriteSADCHit(const char * filename, int n_hits)
{
  // Initialize number generator for simulated channel IDs and ADC values
  std::mt19937 rng(98765);
  std::uniform_int_distribution<int> dist_id(0, 31);     // e.g., 32 possible channels
  std::uniform_int_distribution<int> dist_adc(0, 16383); // e.g., 14-bit ADC

  std::cout << "========== [WRITE] Start writing SADC hits (Self Trigger) ==========\n";

  // Setup DataWriter
  H5DataWriter writer(filename);

  H5SADCHit * wHit = new H5SADCHit();
  // Since AChannel_t is very small, we can buffer many hits before flushing
  wHit->SetBufferCapacity(50000);

  writer.SetData(wHit);
  writer.SetSubrun(555);

  // Start the timer
  auto start_time = std::chrono::high_resolution_clock::now();

  writer.Open();

  // Generate and append individual hit data
  for (int i = 0; i < n_hits; ++i) {
    AChannel_t hit;
    hit.id = static_cast<std::uint16_t>(dist_id(rng));
    hit.adc = static_cast<std::uint32_t>(dist_adc(rng));
    hit.time = static_cast<std::uint64_t>(i) * 200ULL; // Simulated sequential time

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
  // For SADC Hit mode, it's simply the size of AChannel_t times the number of hits.
  double raw_bytes = static_cast<double>(n_hits * sizeof(AChannel_t));
  double uncompressed_mb = raw_bytes / (1024.0 * 1024.0);

  // Avoid division by zero if file creation failed
  double compression_ratio = (file_size_mb > 0.0) ? (uncompressed_mb / file_size_mb) : 0.0;

  double speed_mb_per_sec = file_size_mb / elapsed_sec;
  double speed_hits_per_sec = n_hits / elapsed_sec;

  // Print detailed write statistics
  std::cout << "\n--- Write Statistics Summary ---\n";
  std::cout << "Total Hits Written     : " << n_hits << "\n";
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
void ReadSADCHit(const char * filename)
{
  std::cout << "========== [READ] Read and verify SADC hits ==========\n";

  // Setup DataReader
  H5DataReader reader;
  H5SADCHit * rHit = new H5SADCHit();

  reader.Add(filename);
  reader.SetData(rHit);

  if (!reader.Open()) {
    std::cerr << "Failed to open the file.\n";
    return;
  }

  // GetEntries() represents the total number of individual hits
  int total_entries = reader.GetEntries();
  std::cout << "Total hits to read: " << total_entries << "\n\n";

  // Read and verify data
  int check_interval = 200000; // Print and check every 200,000 hits

  for (int i = 0; i < total_entries; ++i) {
    if (rHit->ReadHit(i) < 0) {
      std::cerr << "Failed to read hit " << i << "!\n";
      break;
    }

    if (i % check_interval == 0 || i == total_entries - 1) {
      // Access the AChannel_t structure directly
      const AChannel_t & hit = rHit->GetHit();

      std::cout << "[Hit Index " << i << "] ID: " << hit.id << ", ADC: " << hit.adc
                << ", Time: " << hit.time << "\n";
    }
  }

  reader.Close();
  std::cout << "--------------------------------------------------\n";
  std::cout << "SADC Hit read test completed successfully.\n";
}

// -----------------------------------------------------------------------------
// Main execution
// -----------------------------------------------------------------------------
int main()
{
  const char * filename = "test_sadc_hits.h5";

  // AChannel_t is small, so we write 1,000,000 hits to generate a measurable payload.
  const int n_hits = 1000000;

  WriteSADCHit(filename, n_hits);
  ReadSADCHit(filename);

  return 0;
}