#include <cstdint>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "HDF5Utils/H5SADCEvent.hh"

int main(int argc, char ** argv)
{
  const char * filename = "sadc_test.h5";
  if (argc > 1) {
    filename = argv[1];
  }

  const int nEvent   = 100000;
  const int nChannel = 32;

  H5DataWriter writer(filename, 4);
  H5SADCEvent sadcEvent;

  sadcEvent.SetBufferEventCapacity(1000);
  sadcEvent.SetBufferMaxBytes(32 * 1024 * 1024);

  writer.SetEvent(&sadcEvent);
  writer.SetSubrun(0);

  if (!writer.Open()) {
    std::cerr << "Failed to open HDF5 file: " << filename << std::endl;
    return 1;
  }

  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<int> adcDist(0, 4095);
  std::uniform_int_distribution<int> tbitDist(0, 0xFFFF);

  auto t_start = std::chrono::high_resolution_clock::now();

  for (int iev = 0; iev < nEvent; ++iev) {
    EventInfo_t info{};
    info.ttype = 3;
    info.nhit  = static_cast<std::uint16_t>(nChannel);
    info.tnum  = static_cast<std::uint32_t>(iev);
    info.ttime = static_cast<std::uint64_t>(1000ull * iev);

    std::vector<AChannel_t> chs;
    chs.reserve(nChannel);

    for (int ich = 0; ich < nChannel; ++ich) {
      AChannel_t ch{};
      ch.id   = static_cast<std::uint16_t>(ich);
      ch.tbit = static_cast<std::uint16_t>(tbitDist(rng));
      ch.adc  = static_cast<std::uint32_t>(adcDist(rng));
      ch.time = static_cast<std::uint32_t>(1000u * iev + static_cast<unsigned>(ich));
      chs.push_back(ch);
    }

    herr_t status = sadcEvent.AppendEvent(info, chs);
    if (status < 0) {
      std::cerr << "AppendEvent failed at event " << iev << std::endl;
      writer.Close();
      return 1;
    }
  }

  auto t_end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> dt = t_end - t_start;
  double real_sec = dt.count();

  double evt_per_sec  = (real_sec > 0.0) ? static_cast<double>(nEvent) / real_sec : 0.0;
  double time_per_evt = (nEvent > 0) ? real_sec / static_cast<double>(nEvent) : 0.0;

  hsize_t memBytes  = writer.GetMemorySize();
  hsize_t fileBytes = writer.GetFileSize();
  double memMB  = static_cast<double>(memBytes)  / (1024.0 * 1024.0);
  double fileMB = static_cast<double>(fileBytes) / (1024.0 * 1024.0);
  double ratio  = (memMB > 0.0) ? (fileMB / memMB) : 0.0;

  std::cout << "\n========== HDF5 SADC Write Performance ==========\n";
  std::cout << "File             : " << filename << "\n";
  std::cout << "Events           : " << nEvent << "\n";
  std::cout << "Channels/event   : " << nChannel << "\n";
  std::cout << "-------------------------------------------\n";
  std::cout << "Write time       : " << real_sec     << " s\n";
  std::cout << "Events / sec     : " << evt_per_sec  << "\n";
  std::cout << "Time / event     : " << time_per_evt << " s\n";
  std::cout << "-------------------------------------------\n";
  std::cout << "Uncompressed mem : " << memMB  << " MB\n";
  std::cout << "File size        : " << fileMB << " MB\n";
  std::cout << "Compression rate : " << ratio * 100.0 << " % (file/mem)\n";

  writer.Close();
  return 0;
}
