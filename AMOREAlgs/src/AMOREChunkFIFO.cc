#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>

#include "AMOREAlgs/AMOREChunkFIFO.hh"

AMOREChunkFIFO::AMOREChunkFIFO()
  : fNChannel(0),
    fHead(0),
    fTail(0),
    fCurrentChunk(nullptr),
    fCurrentSampleIndex(0)
{
}

AMOREChunkFIFO::AMOREChunkFIFO(int nch, int head, int tail)
  : fCurrentChunk(nullptr),
    fCurrentSampleIndex(0)
{
  BookFIFO(nch, head, tail);
}

void AMOREChunkFIFO::BookFIFO(int nch, int head, int tail)
{
  fNChannel = nch;
  fHead = head;
  fTail = tail;
  fCurrentSampleIndex = 0;
  fCurrentChunk = nullptr;
  fLastChunk = nullptr;
  fNextChunk = nullptr;
  fQueue.restart();

  fTotalChunks = 0;
  fTotalSamples = 0;
  fFirstTime = 0;
  fLastTime = 0;
}

// Producer: Push from unpacked data buffers
int AMOREChunkFIFO::PushChunk(unsigned short ** adc, unsigned long * time, int ndp)
{
  if (ndp <= 0) return -1;
  auto chunk = std::make_unique<AMOREChunk>(fNChannel, ndp);
  std::memcpy(chunk->fTime.data(), time, ndp * sizeof(unsigned long));
  for (int ch = 0; ch < fNChannel; ++ch) {
    std::memcpy(chunk->fADC[ch].data(), adc[ch], ndp * sizeof(unsigned short));
  }

  fTotalChunks++;
  fTotalSamples += ndp;
  if (fFirstTime == 0) fFirstTime = chunk->fTime[0];
  fLastTime = chunk->fTime[ndp - 1];

  fQueue.push_back(std::move(chunk));
  return 0;
}

// Producer: Push from raw binary data
int AMOREChunkFIFO::PushChunk(unsigned char * data, int ndp, AMOREADCConf * conf)
{
  if (ndp <= 0 || data == nullptr) return -1;

  auto chunk = std::make_unique<AMOREChunk>(fNChannel, ndp);
  for (int j = 0; j < ndp; j++) {
    const int offset = j * 64;
    for (int i = 0; i < fNChannel; i++) {
      if (conf && conf->ZSU() && conf->PID(i) == 0) {
        chunk->fADC[i][j] = 0;
        continue;
      }
      unsigned int val = data[offset + i * 3] & 0xFF;
      val |= (static_cast<unsigned short>(data[offset + i * 3 + 1] & 0xFF) << 8);
      val |= (static_cast<unsigned short>(data[offset + i * 3 + 2] & 0xFF) << 16);
      chunk->fADC[i][j] = val;
    }
    unsigned long coarsetime = 0;
    for (int k = 0; k < 6; ++k) {
      unsigned long ltmp = data[offset + 48 + k] & 0xFF;
      coarsetime += (ltmp << (k * 8));
    }
    chunk->fTime[j] = coarsetime * 1000; // ns
  }

  fTotalChunks++;
  fTotalSamples += ndp;
  if (fFirstTime == 0) fFirstTime = chunk->fTime[0];
  fLastTime = chunk->fTime[ndp - 1];

  fQueue.push_back(std::move(chunk));

  return 0;
}

int AMOREChunkFIFO::PopCurrent(unsigned short * adc, unsigned long & time)
{
  // 1. Check if we need to rotate chunks because fCurrentChunk is exhausted
  if (fNextChunk && fCurrentChunk && fCurrentSampleIndex >= fCurrentChunk->fTime.size()) {
    fLastChunk = std::move(fCurrentChunk);
    fCurrentChunk = std::move(fNextChunk);
    fCurrentSampleIndex = 0;
    fNextChunk = nullptr;
  }

  // 2. Try to fetch fNextChunk if it's missing or if we're nearing the end of fCurrentChunk
  // (look-ahead for fTail)
  if (!fCurrentChunk || (fCurrentSampleIndex + fTail) > fCurrentChunk->fTime.size()) {
    if (!fNextChunk) {
      // If queue is stopped, do not wait; otherwise, wait for a short timeout
      auto timeout = IsStopped() ? std::chrono::milliseconds(0) : std::chrono::milliseconds(100);
      auto next = fQueue.pop_front(timeout);

      if (next) {
        fNextChunk = std::move(*next);
        // If current is null, initialize it immediately
        if (!fCurrentChunk) {
          fCurrentChunk = std::move(fNextChunk);
          fCurrentSampleIndex = 0;
          fNextChunk = nullptr;
        }
      }
    }
  }

  // 3. Final depletion check
  if (!fCurrentChunk || fCurrentSampleIndex >= fCurrentChunk->fTime.size()) {
    // If current chunk is fully consumed and no next chunk is available, return EOF signal
    if (!fNextChunk) return 1;

    // If fNextChunk became available, rotate and proceed
    fLastChunk = std::move(fCurrentChunk);
    fCurrentChunk = std::move(fNextChunk);
    fCurrentSampleIndex = 0;
    fNextChunk = nullptr;
  }

  // 4. Extract data point
  time = fCurrentChunk->fTime[fCurrentSampleIndex];
  for (int ch = 0; ch < fNChannel; ++ch) {
    adc[ch] = fCurrentChunk->fADC[ch][fCurrentSampleIndex];
  }

  fCurrentSampleIndex++;
  return 0;
}

int AMOREChunkFIFO::DumpCurrent(unsigned short ** outADC, unsigned long * outTime)
{
  if (!fCurrentChunk || !outADC || !outTime) return -1;

  const long totalNDP = static_cast<long>(fHead) + static_cast<long>(fTail);
  if (totalNDP <= 0) return 0;

  long copied = 0;
  const long triggerIdx = static_cast<long>(fCurrentSampleIndex) - 1;
  long startIdx = triggerIdx - static_cast<long>(fHead);

  // 1) Pre-trigger part from fLastChunk
  if (startIdx < 0) {
    const long needFromLast = -startIdx;
    long availFromLast = 0;

    if (fLastChunk) {
      availFromLast = std::min<long>(needFromLast, static_cast<long>(fLastChunk->fTime.size()));
    }

    const long zeroPad = needFromLast - availFromLast;

    // zero-fill missing head part
    if (zeroPad > 0) {
      std::memset(&outTime[copied], 0, zeroPad * sizeof(unsigned long));
      for (int ch = 0; ch < fNChannel; ++ch) {
        std::memset(&outADC[ch][copied], 0, zeroPad * sizeof(unsigned short));
      }
      copied += zeroPad;
    }

    // copy available tail part from fLastChunk
    if (availFromLast > 0) {
      const long lastStart = static_cast<long>(fLastChunk->fTime.size()) - availFromLast;

      std::memcpy(&outTime[copied], &fLastChunk->fTime[lastStart],
                  availFromLast * sizeof(unsigned long));

      for (int ch = 0; ch < fNChannel; ++ch) {
        std::memcpy(&outADC[ch][copied], &fLastChunk->fADC[ch][lastStart],
                    availFromLast * sizeof(unsigned short));
      }
      copied += availFromLast;
    }

    startIdx = 0;
  }

  // 2) Copy from fCurrentChunk
  const long currentSize = static_cast<long>(fCurrentChunk->fTime.size());
  const long availCurrent = std::max<long>(0, currentSize - startIdx);
  const long copyCurrent = std::min<long>(totalNDP - copied, availCurrent);

  if (copyCurrent > 0) {
    std::memcpy(&outTime[copied], &fCurrentChunk->fTime[startIdx],
                copyCurrent * sizeof(unsigned long));

    for (int ch = 0; ch < fNChannel; ++ch) {
      std::memcpy(&outADC[ch][copied], &fCurrentChunk->fADC[ch][startIdx],
                  copyCurrent * sizeof(unsigned short));
    }
    copied += copyCurrent;
  }

  // 3) Copy from fNextChunk
  long remaining = totalNDP - copied;
  if (remaining > 0 && fNextChunk) {
    const long nextSize = static_cast<long>(fNextChunk->fTime.size());
    const long copyNext = std::min<long>(remaining, nextSize);

    if (copyNext > 0) {
      std::memcpy(&outTime[copied], &fNextChunk->fTime[0],
                  copyNext * sizeof(unsigned long));

      for (int ch = 0; ch < fNChannel; ++ch) {
        std::memcpy(&outADC[ch][copied], &fNextChunk->fADC[ch][0],
                    copyNext * sizeof(unsigned short));
      }

      copied += copyNext;
      remaining = totalNDP - copied;
    }
  }

  // 4) Zero-fill tail if still not enough
  if (remaining > 0) {
    std::memset(&outTime[copied], 0, remaining * sizeof(unsigned long));
    for (int ch = 0; ch < fNChannel; ++ch) {
      std::memset(&outADC[ch][copied], 0, remaining * sizeof(unsigned short));
    }
    copied += remaining;
  }

  return static_cast<int>(copied);
}

void AMOREChunkFIFO::DumpStat()
{
  double duration = (fLastTime - fFirstTime) / 1.e9;
  double rate = (duration > 0) ? (fTotalSamples / duration) / 1000.0 : 0;

  std::cout << "\n" << std::setw(40) << std::setfill('=') << "" << std::endl;
  std::cout << " AMORE FIFO FINAL STATISTICS" << std::endl;
  std::cout << std::setw(40) << std::setfill('-') << "" << std::setfill(' ') << std::endl;
  std::cout << " - Runtime      : " << std::fixed << std::setprecision(2) << duration << " sec"
            << std::endl;
  std::cout << " - Total Chunks : " << fTotalChunks << std::endl;
  std::cout << " - Total Samples: " << fTotalSamples << std::endl;
  std::cout << " - Avg Rate     : " << rate << " kHz" << std::endl;
  std::cout << std::setw(40) << std::setfill('=') << "" << std::endl;
}