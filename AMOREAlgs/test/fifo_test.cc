#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <thread>
#include <vector>

// ROOT Headers
#include "TApplication.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TString.h"

// Project Headers
#include "AMOREAlgs/AMOREChunkFIFO.hh"
#include "AMORESystem/AMOREADCConf.hh"

/**
 * SECTION 1: Realistic Data Generation
 * Corrected: Increments timeNs by 10,000 for EVERY sample.
 */
class RawDataGenerator {
public:
  RawDataGenerator(int nch)
    : fNChannel(nch),
      fGen(fRd()),
      fNoiseDist(0, 10.0),
      fUniformDist(0.0, 1.0)
  {
    fBaseline = 100.0;
    fSamplingRate = 100000.0; // 100 kHz
    fSignalRate = 0.1;        // 0.1 Hz per channel
    fSigmaSamples = 0.020 * fSamplingRate;
  }

  void FillChunk(unsigned char * buffer, int ndp, unsigned long startTick)
  {
    for (int j = 0; j < ndp; j++) {
      int offset = j * 64;
      unsigned long currentTick = startTick + j;

      // 1. ADC Data Generation
      for (int i = 0; i < fNChannel; i++) {
        double val = fBaseline + fNoiseDist(fGen);
        if (fUniformDist(fGen) < (fSignalRate / fSamplingRate)) {
          SignalInfo sig;
          sig.startTime = currentTick;
          sig.amplitude = fUniformDist(fGen) * 10000.0;
          fActiveSignals[i].push_back(sig);
        }
        auto & sigs = fActiveSignals[i];
        for (auto it = sigs.begin(); it != sigs.end();) {
          double t = (double)(currentTick - it->startTime);
          double pulse = it->amplitude * std::exp(-(t * t) / (2.0 * fSigmaSamples * fSigmaSamples));
          val += pulse;
          if (t > 5.0 * fSigmaSamples) it = sigs.erase(it);
          else ++it;
        }
        unsigned short adcVal = static_cast<unsigned short>(std::clamp(val, 0.0, 16383.0));
        buffer[offset + i * 3] = (adcVal & 0xFF);
        buffer[offset + i * 3 + 1] = ((adcVal >> 8) & 0xFF);
        buffer[offset + i * 3 + 2] = ((adcVal >> 16) & 0xFF);
      }

      // 2. Timestamp Generation: 10,000 ns per sample (100kHz)
      unsigned long timeNs = currentTick * 10000;
      for (int k = 0; k < 6; k++) {
        buffer[offset + 48 + k] = (unsigned char)((timeNs >> (k * 8)) & 0xFF);
      }
    }
  }

private:
  struct SignalInfo {
    unsigned long startTime;
    double amplitude;
  };
  int fNChannel;
  double fBaseline, fSamplingRate, fSignalRate, fSigmaSamples;
  std::random_device fRd;
  std::mt19937 fGen;
  std::normal_distribution<double> fNoiseDist;
  std::uniform_real_distribution<double> fUniformDist;
  std::vector<SignalInfo> fActiveSignals[64];
};

/**
 * SECTION 2: Producer Thread
 */
void Producer(AMOREChunkFIFO & fifo, int nch, int numChunks, int ndp)
{
  AMOREADCConf conf;
  conf.SetZSU(0);
  for (int i = 0; i < nch; ++i)
    conf.SetPID(i, i + 1);
  RawDataGenerator gen(nch);
  std::vector<unsigned char> rawBuffer(ndp * 64);

  unsigned long cumulativeTicks = 0;
  for (int c = 0; c < numChunks; ++c) {
    if (fifo.IsStopped()) break;
    gen.FillChunk(rawBuffer.data(), ndp, cumulativeTicks);
    fifo.PushChunk(rawBuffer.data(), ndp, &conf);
    cumulativeTicks += ndp; // Next chunk starts exactly after the last sample
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  fifo.Stop();
}

/**
 * SECTION 3: Consumer Thread with Real-time Continuity Monitor
 */
void Consumer(AMOREChunkFIFO & fifo, int nch, int head, int tail)
{
  const double baseline = 100.0;
  const double threshold = 400.0;
  const int deadTimeSamples = 100000;
  const long long expectedStepNs = 10000; // 10,000 ns

  unsigned long lastTime = 0;
  bool isFirstSample = true;
  unsigned long totalMissingSamples = 0;

  bool isTriggered = false;
  int triggerCooldown = 0;
  int eventCount = 0;

  int totalNDP = head + tail;
  std::vector<unsigned long> dumpTime(totalNDP);
  std::vector<unsigned short *> dumpADC(nch);
  for (int i = 0; i < nch; ++i)
    dumpADC[i] = new unsigned short[totalNDP];
  std::vector<unsigned short> currentADC(nch);
  unsigned long currentTime;

  TCanvas * c1 = new TCanvas("c1", "AMORE Stream Monitor", 1400, 900);
  c1->Divide(4, 4);

  while (true) {
    if (fifo.PopCurrent(currentADC.data(), currentTime) == 0) {

      // REAL-TIME INTEGRITY CHECK
      if (!isFirstSample) {
        long long delta = (long long)currentTime - (long long)lastTime;
        if (delta != expectedStepNs) {
          long long lost = (delta / expectedStepNs) - 1;
          if (lost != 0) {
            std::cerr << "[NS ERROR] Gap! Last: " << lastTime << " Now: " << currentTime << " Gap: " << delta
                      << " ns | Lost: " << lost << std::endl;
            if (lost > 0) totalMissingSamples += lost;
          }
        }
      }
      isFirstSample = false;
      lastTime = currentTime;

      // TRIGGER LOGIC
      bool fire = false;
      for (int i = 0; i < nch; ++i) {
        if ((double)currentADC[i] - baseline > threshold) {
          fire = true;
          break;
        }
      }

      if (fire && !isTriggered) {
        eventCount++;
        int copied = fifo.DumpCurrent(dumpADC.data(), dumpTime.data());
        if (copied > 0) {
          for (int i = 0; i < nch; ++i) {
            c1->cd(i + 1);
            TGraph * gr = new TGraph(copied);
            for (int j = 0; j < copied; ++j) {
              double relTimeMs = (double)(dumpTime[j] - dumpTime[0]) / 1000000.0;
              gr->SetPoint(j, relTimeMs, (double)dumpADC[i][j]);
            }
            gr->SetTitle(Form("CH %d (Evt %d);Time (ms);ADC", i, eventCount));
            gr->SetLineColor(kAzure + 2);
            // gr->GetYaxis()->SetRangeUser(0, 16384);
            gr->Draw("AL");
          }
          c1->Update();
          std::cout << ">> Action: [c] Continue, [q] Quit: ";
          std::string choice;
          std::cin >> choice;
          if (choice == "q" || choice == "Q") {
            fifo.Stop();
            break;
          }
        }
        isTriggered = true;
        triggerCooldown = deadTimeSamples;
      }
      if (isTriggered) {
        if (triggerCooldown > 0) triggerCooldown--;
        else isTriggered = false;
      }
    }
    else {
      if (fifo.IsStopped() && fifo.Empty()) break;
      std::this_thread::yield();
    }
  }
  for (int i = 0; i < nch; ++i)
    delete[] dumpADC[i];
  std::cout << "[REPORT] Stream monitoring finished. Total lost samples: " << totalMissingSamples << std::endl;
}

/**
 * SECTION 4: Main
 */
int main(int argc, char ** argv)
{
  TApplication app("app", &argc, argv);
  const int nch = 16;
  const int ndp = 10000;
  const int head = 4000;
  const int tail = 6000;
  const int numChunks = 5000;

  AMOREChunkFIFO fifo(nch, head, tail);
  std::thread t1(Producer, std::ref(fifo), nch, numChunks, ndp);
  std::thread t2(Consumer, std::ref(fifo), nch, head, tail);

  t1.join();
  t2.join();
  return 0;
}