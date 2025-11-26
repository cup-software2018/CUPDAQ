#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "HDF5Utils/H5FADCEvent.hh"

int main(int argc, char ** argv)
{
  const char * filename = "fadc_rand_ch.h5";
  if (argc > 1) { filename = argv[1]; }

  const int nEvent = 10000;
  const int nChannel = 8; // 총 채널
  const int ndp = 1024;    // waveform length

  if (ndp > kH5FADCNDPMAX) {
    std::cerr << "ndp (" << ndp << ") exceeds kH5FADCNDPMAX (" << kH5FADCNDPMAX << ")\n";
    return 1;
  }

  // HDF5 writer
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

  std::random_device rd;
  std::mt19937 rng(rd());

  std::uniform_int_distribution<int> ampDist(0, 4095);
  std::uniform_int_distribution<int> nhitDist(0, nChannel); // 트리거 채널 수
  std::uniform_int_distribution<int> pickDist(0, nChannel - 1);

  std::vector<std::uint16_t> wave(ndp);

  const int ped = 100;
  const double center = 0.5 * (ndp - 1);
  const double sigma = ndp / 10.0;

  using clock = std::chrono::steady_clock;
  auto t_start = clock::now();

  for (int iev = 0; iev < nEvent; ++iev) {

    // 랜덤하게 trigger된 채널 수
    int nhit = nhitDist(rng);
    if (nhit > nChannel) nhit = nChannel;

    // 중복 없는 랜덤 채널 선택
    std::unordered_set<int> fired;
    while ((int)fired.size() < nhit) {
      fired.insert(pickDist(rng));
    }

    EventInfo_t info{};
    info.ttype = 1;
    info.nhit = static_cast<std::uint16_t>(nhit);
    info.tnum = static_cast<std::uint32_t>(iev);
    info.ttime = static_cast<std::uint64_t>(1000ull * iev);

    std::vector<FChannel_t> chs;
    chs.reserve(nhit);

    for (int ich : fired) {
      int amp = ampDist(rng);

      // waveform 생성
      for (int i = 0; i < ndp; ++i) {
        double x = (i - center) / sigma;
        double v = ped + amp * std::exp(-0.5 * x * x);
        if (v < 0) v = 0;
        if (v > 4095) v = 4095;
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

  auto t_end = clock::now();
  double dt = std::chrono::duration<double>(t_end - t_start).count();
  double per_event = dt / double(nEvent);
  double eps = double(nEvent) / dt;

  hsize_t memBytes = writer.GetMemorySize();
  hsize_t fileBytes = writer.GetFileSize();
  double memMB = double(memBytes) / (1024 * 1024);
  double fileMB = double(fileBytes) / (1024 * 1024);
  double ratio = (memMB > 0 ? fileMB / memMB : 0);

  std::cout << "\n========== HDF5 FADC Random Channel Write Performance ==========\n";
  std::cout << "File             : " << filename << "\n";
  std::cout << "Events           : " << nEvent << "\n";
  std::cout << "Channels total   : " << nChannel << "\n";
  std::cout << "Random nhit 0-" << nChannel << "\n";
  std::cout << "NDP              : " << ndp << "\n";
  std::cout << "-------------------------------------------\n";
  std::cout << "Write time       : " << dt << " s\n";
  std::cout << "Events / sec     : " << eps << "\n";
  std::cout << "Time / event     : " << per_event << " s/event\n";
  std::cout << "-------------------------------------------\n";
  std::cout << "Uncompressed mem : " << memMB << " MB\n";
  std::cout << "File size        : " << fileMB << " MB\n";
  std::cout << "Compression rate : " << ratio * 100.0 << " % (file/mem)\n";

  writer.Close();
  return 0;
}
