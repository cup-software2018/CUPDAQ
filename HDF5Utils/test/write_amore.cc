#include <cstdint>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "HDF5Utils/H5AMOREEvent.hh"

int main(int argc, char ** argv)
{
  const char * filename = "amore_test.h5";
  if (argc > 1) {
    filename = argv[1];
  }

  // ========= TEST CONFIG ==========
  const int nEvent   = 100;
  const int nCrystal = 4;
  const int ndp      = 10000;
  // =================================

  if (ndp > kH5AMORENDPMAX) {
    std::cerr << "ndp (" << ndp << ") exceeds kH5AMORENDPMAX (" << kH5AMORENDPMAX << ")\n";
    return 1;
  }

  H5DataWriter writer(filename, 4);  // compression level 4
  H5AMOREEvent amoreEvent;

  amoreEvent.SetNDP(ndp);
  amoreEvent.SetBufferEventCapacity(20);        // flush every 20 events
  amoreEvent.SetBufferMaxBytes(64 * 1024 * 1024); // also flush when > 64 MB

  writer.SetEvent(&amoreEvent);
  writer.SetSubrun(0);

  if (!writer.Open()) {
    std::cerr << "Failed to open HDF5 file: " << filename << std::endl;
    return 1;
  }

  // allocate temp waveforms
  std::vector<std::uint16_t> wavePn(ndp);
  std::vector<std::uint16_t> wavePt(ndp);

  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<int> ampDistPn(0, 4095);
  std::uniform_int_distribution<int> ampDistPt(0, 4095);

  const int pedPn = 100;
  const int pedPt = 100;
  const double center = 0.5 * (ndp - 1);
  const double sigma  = ndp / 20.0;

  // ===== Timer start =====
  auto t_start = std::chrono::high_resolution_clock::now();

  for (int iev = 0; iev < nEvent; ++iev) {
    EventInfo_t info{};
    info.ttype = 2;
    info.nhit  = static_cast<std::uint16_t>(nCrystal);
    info.tnum  = static_cast<std::uint32_t>(iev);
    info.ttime = static_cast<std::uint64_t>(1000ull * iev);

    std::vector<Crystal_t> crystals;
    crystals.reserve(nCrystal);

    for (int icr = 0; icr < nCrystal; ++icr) {
      int ampPn = ampDistPn(rng);
      int ampPt = ampDistPt(rng);

      for (int i = 0; i < ndp; ++i) {
        double x = (i - center) / sigma;

        double vPn = pedPn + static_cast<double>(ampPn) * std::exp(-0.5 * x * x);
        double vPt = pedPt + static_cast<double>(ampPt) * std::exp(-0.5 * x * x);

        if (vPn < 0.0) vPn = 0.0;
        if (vPn > 4095.0) vPn = 4095.0;
        if (vPt < 0.0) vPt = 0.0;
        if (vPt > 4095.0) vPt = 4095.0;

        wavePn[i] = static_cast<std::uint16_t>(vPn);
        wavePt[i] = static_cast<std::uint16_t>(vPt);
      }

      Crystal_t c;
      c.id    = static_cast<std::uint16_t>(icr);
      c.ttime = static_cast<std::uint64_t>(1000ull * iev);
      c.SetWaveforms(wavePn.data(), wavePt.data(), ndp);

      crystals.push_back(c);
    }

    herr_t status = amoreEvent.AppendEvent(info, crystals);
    if (status < 0) {
      std::cerr << "AppendEvent failed at event " << iev << std::endl;
      writer.Close();
      return 1;
    }
  }

  // ===== Timer stop =====
  auto t_end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> dt = t_end - t_start;
  double real_sec = dt.count();

  double evt_per_sec  = (real_sec > 0 ? double(nEvent) / real_sec : 0.0);
  double time_per_evt = (nEvent > 0 ? real_sec / double(nEvent) : 0.0);

  // ===== Compression statistics =====
  hsize_t memBytes  = writer.GetMemorySize();
  hsize_t fileBytes = writer.GetFileSize();
  double memMB  = double(memBytes)  / (1024.0 * 1024.0);
  double fileMB = double(fileBytes) / (1024.0 * 1024.0);
  double ratio  = (memMB > 0.0 ? fileMB / memMB : 0.0);

  std::cout << "\n========== HDF5 AMORE Write Performance ==========\n";
  std::cout << "File             : " << filename << "\n";
  std::cout << "Events           : " << nEvent << "\n";
  std::cout << "Crystals/event   : " << nCrystal << "\n";
  std::cout << "NDP              : " << ndp << "\n";
  std::cout << "-------------------------------------------\n";
  std::cout << "Write time       : " << real_sec << " s\n";
  std::cout << "Events / sec     : " << evt_per_sec  << "\n";
  std::cout << "Time / event     : " << time_per_evt << " s\n";
  std::cout << "-------------------------------------------\n";
  std::cout << "Uncompressed mem : " << memMB  << " MB\n";
  std::cout << "File size        : " << fileMB << " MB\n";
  std::cout << "Compression rate : " << ratio * 100.0 << " % (file/mem)\n";

  writer.Close();
  return 0;
}
