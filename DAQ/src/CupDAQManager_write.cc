#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <string>

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"

void CupDAQManager::TF_WriteEvent()
{
  fWriteStatus.store(READY);

  if (!WaitRunState(fRunStatus, RUNSTATE::kRUNNING, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }

  const char * adcmode = (fADCMode == ADC::SMODE) ? "SADC mode" : "FADC mode";
  INFO("started as %s", adcmode);

  if (!OpenNewOutputFile()) {
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  if (fADCMode == ADC::SMODE) {
    switch (fOutputFileFormat) {
      case OUTPUT::ROOT: WriteSADC_MOD_ROOT(); break;
      case OUTPUT::HDF5: WriteSADC_MOD_HDF5(); break;
      case OUTPUT::GZIP: WriteSADC_MOD_GZIP(); break;
      default: break;
    }
  }
  else {
    switch (fOutputFileFormat) {
      case OUTPUT::ROOT: WriteFADC_MOD_ROOT(); break;
      case OUTPUT::HDF5: WriteFADC_MOD_HDF5(); break;
      case OUTPUT::GZIP: WriteFADC_MOD_GZIP(); break;
      default: break;
    }
  }

  {
    std::lock_guard<std::mutex> lock(fWriteFileMutex);
    if (fROOTFile && fROOTFile->IsOpen()) {
      fROOTFile->cd();
      fROOTTree->Write();

      fTotalWrittenDataSize += fROOTFile->GetEND();
      std::string fname = fROOTFile->GetName();

      fROOTFile->Close();
      INFO("output data %s closed", fname.c_str());
    }
  }

  CloseHDF5Output();

  fWriteStatus.store(ENDED);

  // build already ended, clear fBuiltEventBuffer1
  if (!fBuiltEventBuffer1.empty()) {
    INFO("fBuiltEventBuffer1 is not empty, clearing");
    fBuiltEventBuffer1.clear();
  }

  INFO("ended");
}

bool CupDAQManager::OpenNewOutputFile()
{
  int nfile = fOutputFileList.size();

  if (nfile == 0) {
    std::string fname;

    if (fOutputFilename.empty()) {
      std::string extension;
      switch (fOutputFileFormat) {
        case OUTPUT::ROOT: extension = "root"; break;
        case OUTPUT::HDF5: extension = "h5"; break;
        case OUTPUT::GZIP: extension = "gz"; break;
        default: break;
      }

      std::string adcname = fDAQName;

      // Lambda helper for string replacement
      auto replace_all = [](std::string & str, const std::string & from, const std::string & to) {
        std::size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
          str.replace(start_pos, from.length(), to);
          start_pos += to.length();
        }
      };

      replace_all(adcname, "DAQ", "");
      replace_all(adcname, "MERGER", "");

      // Format run number as %06d safely
      char run_str[16];
      std::snprintf(run_str, sizeof(run_str), "%06d", fRunNumber);

      const char * rawdata_dir_env = std::getenv("RAWDATA_DIR");
      if (!rawdata_dir_env) {
        WARNING("RAWDATA_DIR is not set");
        fname = adcname + "_" + std::string(run_str) + "." + extension;
      }
      else {
        // Use C++17 std::filesystem for safe and fast directory creation
        namespace fs = std::filesystem;
        fs::path dirname = fs::path(rawdata_dir_env) / "RAW" / run_str;

        if (!fs::exists(dirname)) {
          fs::create_directories(dirname);
          INFO("%s created", dirname.c_str());
        }
        else {
          INFO("%s already exist", dirname.c_str());
        }

        fs::path full_path = dirname / (adcname + "_" + std::string(run_str) + "." + extension);
        fname = full_path.string();
      }
      fOutputFilename = fname;
    }
    else {
      // Extract BaseName
      std::size_t slash_pos = fOutputFilename.find_last_of("/\\");
      fname = (slash_pos == std::string::npos) ? fOutputFilename
                                               : fOutputFilename.substr(slash_pos + 1);

      // Determine format from extension
      if (fname.find(".root") != std::string::npos) fOutputFileFormat = OUTPUT::ROOT;
      else if (fname.find(".h5") != std::string::npos || fname.find(".hdf") != std::string::npos)
        fOutputFileFormat = OUTPUT::HDF5;
      else if (fname.find(".gz") != std::string::npos || fname.find(".dat") != std::string::npos)
        fOutputFileFormat = OUTPUT::GZIP;
      else {
        WARNING("output file format is unclear");
        INFO("output file is going to be written by ROOT");
        fOutputFileFormat = OUTPUT::ROOT;
      }
    }
  }

  // Append sub-file number (e.g., .00000)
  char filename_buf[512];
  std::snprintf(filename_buf, sizeof(filename_buf), "%s.%05d", fOutputFilename.c_str(), nfile);
  std::string filename = filename_buf;

  long retval = 0;
  std::unique_lock<std::mutex> lock(fWriteFileMutex);

  switch (fOutputFileFormat) {
    case OUTPUT::ROOT: retval = OpenNewROOTFile(filename.c_str()); break;
    case OUTPUT::HDF5: retval = OpenNewHDF5File(filename.c_str()); break;
    case OUTPUT::GZIP: retval = OpenNewGZIPFile(filename.c_str()); break;
    default: break;
  }

  lock.unlock();

  if (retval < 0) return false;

  fTotalWrittenDataSize += retval;
  fOutputFileList.push_back(filename.c_str());

  return true;
}

#ifdef ENABLE_HDF5

void CupDAQManager::CloseHDF5Output()
{
  if (fHDF5File) {
    if (fHDF5File->IsOpen()) {
      fTotalWrittenDataSize += fHDF5File->GetFileSize();
      const char * fname = fHDF5File->GetFilename();
      fHDF5File->Close();
      INFO("output data %s closed", fname);
    }
    delete fHDF5File;
    fHDF5File = nullptr;
  }

  if (fH5Event) {
    delete fH5Event;
    fH5Event = nullptr;
  }
}

#else

void CupDAQManager::CloseHDF5Output()
{
  // Do nothing
}

#endif