#pragma once

#include <cstring>
#include <memory>
#include <vector>

#include "TObject.h"

#include "AMOREDAQ/AMOREADCConf.hh"
#include "DAQUtils/ConcurrentDeque.hh"

struct ChunkData {
  std::vector<unsigned long> fTime;
  std::vector<std::vector<unsigned int>> fADC;

  ChunkData(int nch, int ndp)
  {
    fTime.resize(ndp);
    fADC.assign(nch, std::vector<unsigned int>(ndp));
  }
};

class ChunkDataFIFO : public TObject {
public:
  ChunkDataFIFO();
  ChunkDataFIFO(int nch, int head, int tail);
  virtual ~ChunkDataFIFO() = default;

  void BookFIFO(int nch, int head, int tail);

  // Producer methods
  int PushChunk(unsigned int ** adc, unsigned long * time, int ndp);
  int PushChunk(unsigned char * data, int ndp, AMOREADCConf * conf);

  // Consumer methods
  int PopCurrent(unsigned int * adc, unsigned long & time);
  int DumpCurrent(unsigned int ** outADC, unsigned long * outTime);

  // Controls and status
  void Stop();
  void Restart();
  bool IsStopped() const;
  bool Empty() const;
  std::size_t GetQueueSize() const;

private:
  int fNChannel;
  int fHead; // Head window size
  int fTail; // Tail window size

  ConcurrentDeque<std::unique_ptr<ChunkData>> fQueue;

  std::unique_ptr<ChunkData> fLastChunk;
  std::unique_ptr<ChunkData> fCurrentChunk;
  std::unique_ptr<ChunkData> fNextChunk;
  size_t fCurrentSampleIndex;

  ClassDef(ChunkDataFIFO, 0)
};

inline void ChunkDataFIFO::Stop() { fQueue.stop(); }
inline void ChunkDataFIFO::Restart() { fQueue.restart(); }
inline bool ChunkDataFIFO::IsStopped() const { return fQueue.is_stopped(); }
inline bool ChunkDataFIFO::Empty() const { return fQueue.empty() && !fCurrentChunk; }
inline std::size_t ChunkDataFIFO::GetQueueSize() const { return fQueue.size(); }