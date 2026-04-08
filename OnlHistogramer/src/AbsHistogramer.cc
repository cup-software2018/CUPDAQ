#include <cstdio>

// Removed TBranch/TTree includes

#include "DAQUtils/ELog.hh"
#include "OnlHistogramer/AbsHistogramer.hh"

AbsHistogramer::AbsHistogramer() { fHistProxy = new HistProxy(); }

AbsHistogramer::~AbsHistogramer() { delete fHistProxy; }

bool AbsHistogramer::Open()
{
  int nfile = static_cast<int>(fROOTFileList.size());

  char fname_buf[512];
  std::snprintf(fname_buf, sizeof(fname_buf), "%s.%05d", fROOTFilename.c_str(), nfile);
  std::string fname(fname_buf);

  // Instead of opening a TFile immediately, we start the THttpServer
  // We only run one server. If nfile == 0, we create it.
  if (nfile == 0) {
    int port = 8080;
    fHttpServer = new THttpServer(Form("http:%d", port));
    fHttpServer->Register("/", fHistProxy);
    INFO("THttpServer started on port %d", port);
  }

  // Just register the planned filename
  fROOTFileList.push_back(fname);
  return true;
}

void AbsHistogramer::Close()
{
  if (!fROOTFileList.empty()) {
    std::string fname = fROOTFileList.back();
    fROOTFile = new TFile(fname.c_str(), "recreate", "", 0);
    if (fROOTFile->IsOpen()) {
      fROOTFile->cd();
      int nobj = fHistProxy->GetEntries();
      for (int i = 0; i < nobj; i++) {
        TObject * obj = fHistProxy->At(i);
        obj->Write();
      }
      fROOTFile->Close();
      INFO("%s histogram file has been written completely.", fname.c_str());
    } else {
      WARNING("Failed to create the final histogram file %s", fname.c_str());
    }
    delete fROOTFile;
    fROOTFile = nullptr;
  }

  if (fHttpServer) {
    delete fHttpServer;
    fHttpServer = nullptr;
  }
}

void AbsHistogramer::Update()
{
  if (fHttpServer) {
    fHttpServer->ProcessRequests();
  }
}