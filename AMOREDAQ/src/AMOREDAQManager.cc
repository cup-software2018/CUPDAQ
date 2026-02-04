#include <fstream>
#include <limits>    // Required for std::numeric_limits
#include <stdexcept> // For std::stoi exceptions
#include <string>
#include <thread>
#include <vector>

#include "AMOREDAQ/AMOREADC.hh"
#include "AMOREDAQ/AMOREADCConf.hh"
#include "AMOREDAQ/AMOREDAQManager.hh"
#include "AMOREDAQ/AMORETCBConf.hh"
#include "DAQConfig/DAQConf.hh"
#include "DAQUtils/ELog.hh"

ClassImp(AMOREDAQManager)

AMOREDAQManager::AMOREDAQManager()
  : CupDAQManager(),
    fTCB(AMORETCB::Instance())
{
  fConfigList = new AbsConfList();
}

AMOREDAQManager::~AMOREDAQManager() { delete fConfigList; }

bool AMOREDAQManager::ReadConfig()
{
  if (fConfigFilename.IsNull()) {
    ERROR("config filename not set");
    return false;
  }

  std::ifstream input;
  input.open(fConfigFilename.Data(), std::ifstream::in);

  if (!input.is_open()) {
    ERROR("Failed to open config file: %s", fConfigFilename.Data());
    return false;
  }

  if (!ParseConfig(input)) { return false; }

  INFO("reading config file %s done.", fConfigFilename.Data());

  return true;
}

bool AMOREDAQManager::ParseConfig(std::ifstream & file)
{
  auto GetNextToken = [&](std::string & token) -> bool {
    while (file >> token) {
      // If token starts with '#', ignore the rest of the line and loop again
      if (token.empty()) continue;
      if (token[0] == '#') {
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        continue;
      }
      return true; // Found a valid token
    }
    return false; // End of file
  };

  auto GetNextInt = [&](int & value, const std::string & contextKey) -> bool {
    std::string token;
    if (!GetNextToken(token)) {
      ERROR("Unexpected EOF while expecting value for %s", contextKey.c_str());
      return false;
    }
    try {
      value = std::stoi(token);
    }
    catch (const std::exception & e) {
      ERROR("Invalid integer format '%s' for key %s", token.c_str(), contextKey.c_str());
      return false;
    }
    return true;
  };

  std::string token;

  // Main parsing loop
  while (GetNextToken(token)) {
    if (token == "DAQ") {
      auto * daq = static_cast<DAQConf *>(fConfigList->GetDAQConfig());
      if (!daq) {
        daq = new DAQConf();
        fConfigList->Add(daq);
      }

      std::string key;
      bool blockEnded = false;
      while (GetNextToken(key)) {
        if (key == "END") {
          blockEnded = true;
          break;
        }

        if (key == "SERVER") {
          int id = 0;
          int port = 0;
          std::string name, ip;

          if (!GetNextInt(id, "DAQ SERVER ID")) return false;
          if (!GetNextToken(name)) return false;
          if (!GetNextToken(ip)) return false;
          if (!GetNextInt(port, "DAQ SERVER PORT")) return false;

          daq->AddDAQ(id, name, ip, port);
        }
      }

      if (!blockEnded) {
        ERROR("Missing 'END' for DAQ block");
        return false;
      }
    }
    else if (token == "AMORETCB") {
      auto * tcb = new AMORETCBConf();

      std::string key;
      bool blockEnded = false;

      while (GetNextToken(key)) {
        if (key == "END") {
          blockEnded = true;
          break;
        }

        int val = 0;
        // Check key and read corresponding value
        if (key == "CW") {
          if (!GetNextInt(val, "CW")) return false;
          tcb->SetCW(val);
        }
        else if (key == "DT") {
          if (!GetNextInt(val, "DT")) return false;
          tcb->SetDT(val);
        }
        else if (key == "SCALE") {
          if (!GetNextInt(val, "SCALE")) return false;
          tcb->SetSCALE(val);
        }
        else {
          WARNING("Unknown key in AMORETCB: %s", key.c_str());
          // return false; // Uncomment if strict check is required
        }
      }

      if (!blockEnded) {
        ERROR("Missing 'END' for AMORETCB block");
        return false;
      }
      fConfigList->Add(tcb);
    }
    else if (token == "AMOREADC") {
      int sid = 0;
      int nch = 0;

      // Read SID and NCH using helper
      if (!GetNextInt(sid, "AMOREADC SID") || !GetNextInt(nch, "AMOREADC NCH")) { return false; }

      auto * adc = new AMOREADCConf(sid);
      adc->SetNCH(nch);

      std::string key;
      bool blockEnded = false;

      while (GetNextToken(key)) {
        if (key == "END") {
          blockEnded = true;
          break;
        }

        // Arrays: Read 'nch' values
        if (key == "CID" || key == "PID" || key == "TRGON") {
          for (int i = 0; i < nch; ++i) {
            int val = 0;
            // Use key + index for error context
            std::string ctx = key + " index " + std::to_string(i);
            if (!GetNextInt(val, ctx)) return false;

            if (key == "CID") adc->SetCID(i, val);
            else if (key == "PID") adc->SetPID(i, val);
            else if (key == "TRGON") adc->SetTRGON(i, val);
          }
        }
        // Single values
        else {
          int val = 0;
          if (key == "SR") {
            if (!GetNextInt(val, "SR")) return false;
            adc->SetSR(val);
          }
          else if (key == "RL") {
            if (!GetNextInt(val, "RL")) return false;
            adc->SetRL(val);
          }
          else if (key == "DLY") {
            if (!GetNextInt(val, "DLY")) return false;
            adc->SetDLY(val);
          }
          else if (key == "ZSU") {
            if (!GetNextInt(val, "ZSU")) return false;
            adc->SetZSU(val);
          }
          else {
            WARNING("Unknown key in AMOREADC (SID %d): %s", sid, key.c_str());
            // return false;
          }
        }
      }

      if (!blockEnded) {
        ERROR("Missing 'END' for AMOREADC block (SID %d)", sid);
        return false;
      }
      fConfigList->Add(adc);
    }
    else {
      ERROR("Unknown configuration block: %s", token.c_str());
      return false;
    }
  }

  return true;
}

bool AMOREDAQManager::PrepareDAQ() { return true; }
