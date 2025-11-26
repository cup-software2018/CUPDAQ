#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "HDF5Utils/H5FADCEvent.hh"

int main(int argc, char ** argv)
{
  const char * filename = "fadc_test.h5";
  if (argc > 1) { filename = argv[1]; }

  const int nEvent = 10000;
  const int nChannel = 8;
  const int ndp = 1024;

  if (ndp > kH5FADCNDPMAX) {
    std::cerr << "ndp (" << ndp << ") exceeds kH5FADCNDPMAX (" << kH5FADCNDPMAX << ")\n";
    return 1;
  }

  H5DataWriter writer(filename, 1);
  H5FADCEvent fadcEvent;

  fadcEvent.SetNDP(ndp);
  fadcEvent.SetBufferEventCapacity(200);
  fadcEvent.SetBufferMaxBytes(64 * 1024 * 1024);

  writer.SetEvent(&fadcEvent);
  writer.SetSubrun(0);

  if (!writer.Open()) {
    std::cerr << "Failed to open HDF5 file: " << filename << std::endl;
    return 1;
  }

  std::vector<std::uint16_t> wave(ndp);

  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<int> ampDist(0, 4095);

  const int ped = 100;
  const double center = 0.5 * (ndp - 1);
  const double sigma = ndp / 10.0;

  using clock = std::chrono::steady_clock;
  const auto t_start = clock::now();

  for (int iev = 0; iev < nEvent; ++iev) {
    EventInfo_t info{};
    info.ttype = 1;
    info.nhit = static_cast<std::uint16_t>(nChannel);
    info.tnum = static_cast<std::uint32_t>(iev);
    info.ttime = static_cast<std::uint64_t>(1000ull * iev);

    std::vector<FChannel_t> chs;
    chs.reserve(nChannel);

    for (int ich = 0; ich < nChannel; ++ich) {
      int amp = ampDist(rng);

      for (int i = 0; i < ndp; ++i) {
        double x = (i - center) / sigma;
        double v = static_cast<double>(ped) + static_cast<double>(amp) * std::exp(-0.5 * x * x);
        if (v < 0.0) v = 0.0;
        if (v > 4095.) v = 4095.;
        wave[i] = static_cast<std::uint16_t>(v);
      }

      FChannel_t ch;
      ch.id = static_cast<std::uint16_t>(ich);
      ch.tbit = 0;
      ch.ped = static_cast<std::uint16_t>(ped);
      ch.SetWaveform(wave.data(), ndp);

      chs.push_back(ch);
    }

    herr_t status = fadcEvent.AppendEvent(info, chs);
    if (status < 0) {
      std::cerr << "AppendEvent failed at event " << iev << std::endl;
      writer.Close();
      return 1;
    }
  }

  const auto t_end = clock::now();
  const double dt = std::chrono::duration<double>(t_end - t_start).count();
  const double per_event = (nEvent > 0) ? dt / static_cast<double>(nEvent) : 0.0;
  const double evt_per_sec = (dt > 0.0) ? static_cast<double>(nEvent) / dt : 0.0;

  hsize_t memBytes = writer.GetMemorySize();
  hsize_t fileBytes = writer.GetFileSize();
  double memMB = static_cast<double>(memBytes) / (1024.0 * 1024.0);
  double fileMB = static_cast<double>(fileBytes) / (1024.0 * 1024.0);
  double ratio = (memMB > 0.0) ? (fileMB / memMB) : 0.0;

  std::cout << "\n========== HDF5 FADC Write Performance ==========\n";
  std::cout << "File             : " << filename << "\n";
  std::cout << "Events           : " << nEvent << "\n";
  std::cout << "Channels/event   : " << nChannel << "\n";
  std::cout << "NDP              : " << ndp << "\n";
  std::cout << "-------------------------------------------\n";
  std::cout << "Write time       : " << dt << " s\n";
  std::cout << "Events / sec     : " << evt_per_sec << "\n";
  std::cout << "Time / event     : " << per_event << " s/event\n";
  std::cout << "-------------------------------------------\n";
  std::cout << "Uncompressed mem : " << memMB << " MB\n";
  std::cout << "File size        : " << fileMB << " MB\n";
  std::cout << "Compression rate : " << ratio * 100.0 << " % (file/mem)\n";

  // 필요하면 이것도 그대로 둘 수 있음 (포맷만 약간 다름)
  writer.PrintStats();
  writer.Close();
  
  return 0;
}
