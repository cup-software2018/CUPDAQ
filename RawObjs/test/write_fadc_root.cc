#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

#include "TFile.h"
#include "TTree.h"

#include "RawObjs/FChannel.hh"
#include "RawObjs/FChannelData.hh"

int main(int argc, char ** argv)
{
  const char * filename = "fadc_test.root";
  if (argc > 1) { filename = argv[1]; }

  const int nEvent = 100;
  const int nChannel = 8;
  const int ndp = 1024;

  auto * file = TFile::Open(filename, "RECREATE");
  if (!file || file->IsZombie()) {
    std::cerr << "Failed to create ROOT file: " << filename << std::endl;
    return 1;
  }
  file->SetCompressionLevel(1);

  TTree tree("t", "FADC test tree");

  auto * chs = new FChannelData;
  tree.Branch("fch", "FChannelData", &chs, 32000, 0);

  std::vector<unsigned short> wave(ndp);

  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<int> ampDist(0, 4095);

  const int ped = 100;
  const double center = 0.5 * (ndp - 1);
  const double sigma = ndp / 10.0;

  double DT = 0;
  using clock = std::chrono::steady_clock;

  for (int iev = 0; iev < nEvent; ++iev) {
    chs->Clear("C");

    for (int ich = 0; ich < nChannel; ++ich) {
      int amp = ampDist(rng);

      for (int i = 0; i < ndp; ++i) {
        double x = (i - center) / sigma;
        double v = static_cast<double>(ped) + static_cast<double>(amp) * std::exp(-0.5 * x * x);
        if (v < 0.0) v = 0.0;
        if (v > 4095.) v = 4095.;
        wave[i] = static_cast<unsigned short>(v);
      }

      FChannel * ch = chs->Add(static_cast<unsigned short>(ich), ndp, wave.data());
      if (ch) { ch->SetPedestal(static_cast<unsigned short>(ped)); }
    }
    const auto t_start = clock::now();
    tree.Fill();
    const auto t_end = clock::now();
    const double dt = std::chrono::duration<double>(t_end - t_start).count();

    DT += dt;
  }

  // 쓰기 및 파일 닫기
  tree.Write();
  file->Close();
  delete file;
  delete chs;

  const double per_event = (nEvent > 0) ? DT / static_cast<double>(nEvent) : 0.0;

  std::cout << "ROOT IO time: " << DT << " s\n";
  std::cout << "ROOT IO time per event: " << per_event << " s/event\n";

  return 0;
}
