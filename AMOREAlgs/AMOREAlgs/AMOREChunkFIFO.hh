#pragma once

#include <cstring>
#include <memory>
#include <vector>

#include "TObject.h"

#include "AMORESystem/AMOREADCConf.hh"
#include "DAQUtils/ConcurrentDeque.hh"

struct AMOREChunk {
  std::vector<unsigned long> fTime;
  std::vector<std::vector<unsigned int>> fADC;

  AMOREChunk(int nch, int ndp)
  {
    fTime.resize(ndp);
    fADC.assign(nch, std::vector<unsigned int>(ndp));
  }
};

class AMOREChunkFIFO : public TObject {
public:
  AMOREChunkFIFO();
  AMOREChunkFIFO(int nch, int head, int tail);
  virtual ~AMOREChunkFIFO() = default;

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

  ConcurrentDeque<std::unique_ptr<AMOREChunk>> fQueue;

  std::unique_ptr<AMOREChunk> fLastChunk;
  std::unique_ptr<AMOREChunk> fCurrentChunk;
  std::unique_ptr<AMOREChunk> fNextChunk;
  size_t fCurrentSampleIndex;

  ClassDef(AMOREChunkFIFO, 0)
};

inline void AMOREChunkFIFO::Stop() { fQueue.stop(); }
inline void AMOREChunkFIFO::Restart() { fQueue.restart(); }
inline bool AMOREChunkFIFO::IsStopped() const { return fQueue.is_stopped(); }
inline bool AMOREChunkFIFO::Empty() const { return fQueue.empty() && !fCurrentChunk; }
inline std::size_t AMOREChunkFIFO::GetQueueSize() const { return fQueue.size(); }