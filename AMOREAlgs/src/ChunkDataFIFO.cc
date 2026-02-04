#include <algorithm>
#include <chrono>

#include "AMOREAlgs/ChunkDataFIFO.hh"

ClassImp(ChunkDataFIFO)

ChunkDataFIFO::ChunkDataFIFO()
  : TObject(),
    fNChannel(0),
    fHead(0),
    fTail(0),
    fCurrentChunk(nullptr),
    fCurrentSampleIndex(0)
{
}

ChunkDataFIFO::ChunkDataFIFO(int nch, int head, int tail)
  : TObject(),
    fCurrentChunk(nullptr),
    fCurrentSampleIndex(0)
{
  BookFIFO(nch, head, tail);
}

void ChunkDataFIFO::BookFIFO(int nch, int head, int tail)
{
  fNChannel = nch;
  fHead = head;
  fTail = tail;
  fCurrentSampleIndex = 0;
  fCurrentChunk = nullptr;
  fLastChunk = nullptr;
  fNextChunk = nullptr;
  fQueue.restart();
}

// Producer: Push from unpacked data buffers
int ChunkDataFIFO::PushChunk(unsigned int ** adc, unsigned long * time, int ndp)
{
  if (ndp <= 0) return -1;
  auto chunk = std::make_unique<ChunkData>(fNChannel, ndp);
  std::memcpy(chunk->fTime.data(), time, ndp * sizeof(unsigned long));
  for (int ch = 0; ch < fNChannel; ++ch) {
    std::memcpy(chunk->fADC[ch].data(), adc[ch], ndp * sizeof(unsigned int));
  }
  fQueue.push_back(std::move(chunk));
  return 0;
}

// Producer: Push from raw binary data
int ChunkDataFIFO::PushChunk(unsigned char * data, int ndp, AMOREADCConf * conf)
{
  if (ndp <= 0 || data == nullptr) return -1;
  auto chunk = std::make_unique<ChunkData>(fNChannel, ndp);
  for (int j = 0; j < ndp; j++) {
    const int offset = j * 64;
    for (int i = 0; i < fNChannel; i++) {
      if (conf && conf->ZSU() && conf->PID(i) == 0) {
        chunk->fADC[i][j] = 0;
        continue;
      }
      unsigned int val = data[offset + i * 3] & 0xFF;
      val |= (static_cast<unsigned int>(data[offset + i * 3 + 1] & 0xFF) << 8);
      val |= (static_cast<unsigned int>(data[offset + i * 3 + 2] & 0xFF) << 16);
      chunk->fADC[i][j] = val;
    }
    unsigned long coarsetime = 0;
    for (int k = 0; k < 6; ++k) {
      unsigned long ltmp = data[offset + 48 + k] & 0xFF;
      coarsetime += (ltmp << (k * 8));
    }
    chunk->fTime[j] = coarsetime;
  }
  fQueue.push_back(std::move(chunk));
  return 0;
}

int ChunkDataFIFO::PopCurrent(unsigned int * adc, unsigned long & time)
{
  // 1. Check if we need to rotate chunks because fCurrentChunk is exhausted
  if (fNextChunk && fCurrentChunk && fCurrentSampleIndex >= fCurrentChunk->fTime.size()) {
    fLastChunk = std::move(fCurrentChunk);
    fCurrentChunk = std::move(fNextChunk);
    fCurrentSampleIndex = 0;
    fNextChunk = nullptr;
  }

  // 2. Try to fetch fNextChunk if it's missing or if we're nearing the end of fCurrentChunk (look-ahead for fTail)
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

int ChunkDataFIFO::DumpCurrent(unsigned int ** outADC, unsigned long * outTime)
{
  if (!fCurrentChunk) return -1;

  long triggerIdx = (long)fCurrentSampleIndex - 1;
  long startIdx = triggerIdx - fHead;
  long totalNDP = fHead + fTail;
  int copied = 0;

  // Case 1: Start index falls into fLastChunk (Pre-trigger data)
  if (startIdx < 0) {
    int headPartNDP = -startIdx;
    if (fLastChunk) {
      long lastStart = (long)fLastChunk->fTime.size() - headPartNDP;
      std::memcpy(&outTime[0], &fLastChunk->fTime[lastStart], headPartNDP * sizeof(unsigned long));
      for (int ch = 0; ch < fNChannel; ++ch)
        std::memcpy(&outADC[ch][0], &fLastChunk->fADC[ch][lastStart], headPartNDP * sizeof(unsigned int));
    }
    else {
      // No LastChunk available: fill with zeros
      std::memset(&outTime[0], 0, headPartNDP * sizeof(unsigned long));
      for (int ch = 0; ch < fNChannel; ++ch)
        std::memset(&outADC[ch][0], 0, headPartNDP * sizeof(unsigned int));
    }
    copied += headPartNDP;
    startIdx = 0;
  }

  // Case 2: Copy from fCurrentChunk
  int currentAvailable = (int)fCurrentChunk->fTime.size() - (int)startIdx;
  int toCopyFromCurrent = std::min((int)totalNDP - copied, currentAvailable);

  if (toCopyFromCurrent > 0) {
    std::memcpy(&outTime[copied], &fCurrentChunk->fTime[startIdx], toCopyFromCurrent * sizeof(unsigned long));
    for (int ch = 0; ch < fNChannel; ++ch)
      std::memcpy(&outADC[ch][copied], &fCurrentChunk->fADC[ch][startIdx], toCopyFromCurrent * sizeof(unsigned int));
    copied += toCopyFromCurrent;
  }

  // Case 3: Copy from fNextChunk (Post-trigger data crossing chunk boundary)
  if (copied < totalNDP) {
    int tailRemaining = totalNDP - copied;

    if (fNextChunk) {
      int toCopyFromNext = std::min(tailRemaining, (int)fNextChunk->fTime.size());
      std::memcpy(&outTime[copied], &fNextChunk->fTime[0], toCopyFromNext * sizeof(unsigned long));
      for (int ch = 0; ch < fNChannel; ++ch)
        std::memcpy(&outADC[ch][copied], &fNextChunk->fADC[ch][0], toCopyFromNext * sizeof(unsigned int));

      copied += toCopyFromNext;
      tailRemaining = totalNDP - copied;
    }

    // Fill remaining with zeros if fNextChunk is insufficient or missing
    if (tailRemaining > 0) {
      std::memset(&outTime[copied], 0, tailRemaining * sizeof(unsigned long));
      for (int ch = 0; ch < fNChannel; ++ch)
        std::memset(&outADC[ch][copied], 0, tailRemaining * sizeof(unsigned int));
      copied += tailRemaining;
    }
  }

  return copied;
}