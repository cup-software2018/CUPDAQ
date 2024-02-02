//
// Created by cupsoft on 7/24/19.
//
#include "DAQ/CupDAQManager.hh"

void CupDAQManager::TF_ReadData()
{
  fReadStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    fLog->Warning("CupDAQManager::TF_ReadData", "exited by exit command");    
    return;
  }
  fLog->Info("CupDAQManager::TF_ReadData", "reading data from ADCs started");

  StartBenchmark("ReadData");
  if (fTriggerMode == TRIGGER::GLOBAL) { ReadData_GLT(); }
  else {
    ReadData_MOD();
  }
  StopBenchmark("ReadData");

  if (fReadStatus != ERROR) fReadStatus = ENDED;
  fLog->Info("CupDAQManager::TF_ReadData", "reading data from ADCs ended");
}

void CupDAQManager::ReadData_GLT()
{
  int nadc = GetEntries();

  auto * theADC = (AbsADC *)fCont[0];
  int * bcounts = new int[nadc];

  double perror = 0;
  double integral = 0;

  bool endsleep = false;

  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  fReadStatus = RUNNING;
  while (true) {
    // for emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;

    if (RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) && !endsleep) {
      fLog->Info("CupDAQManager::ReadData_GLT",
                 "waiting for reading remaining data in ADCs");
      gSystem->Sleep(1000);
      endsleep = true;
    }

    int bcount = ReadBCountMin(bcounts);
    if (bcount < 0) {
      RUNSTATE::SetError(fRunStatus);
      fReadStatus = ERROR;
      break;
    }
    if (endsleep && fDoEndRun && bcount < fMinimumBCount) {
      fLog->Info("CupDAQManager::ReadData_GLT",
                 "no more data in ADCs [bcount=%d]", bcount);
      break;
    }

    int n = bcount / fMinimumBCount;
    int size = n;

    if (n > 16) n = 16;

    bcount = n * fMinimumBCount;

    if (bcount > 0) {
      for (int i = 0; i < nadc; i++) {
        if (ReadADCData(i, bcount) < 0) {
          RUNSTATE::SetError(fRunStatus);
          fReadStatus = ERROR;
          break;
        }
      }

      if (RUNSTATE::CheckError(fRunStatus)) break;

      mlock.lock();
      fTriggerNumber = theADC->GetCurrentTrgNumber();
      fCurrentTime = theADC->GetCurrentTime();
      fTriggerTime = fCurrentTime;
      for (int i = 0; i < nadc; i++) {
        fRemainingBCount[i] = bcounts[i] - bcount;
      }
      mlock.unlock();
      fTotalReadDataSize += nadc * bcount * kKILOBYTES;
    }

    ThreadSleep(fReadSleep, perror, integral, size, 16);
  }

  delete[] bcounts;
}

void CupDAQManager::ReadData_MOD() {}
