#include <cstdio>

#include "TBranch.h"
#include "TBranchRef.h"
#include "TTree.h"

#include "OnlHistogramer/AbsHistogramer.hh"

AbsHistogramer::AbsHistogramer() { fHistProxy = new HistProxy(); }

AbsHistogramer::~AbsHistogramer() { delete fHistProxy; }

bool AbsHistogramer::Open()
{
  int nfile = static_cast<int>(fROOTFileList.size());

  char fname_buf[512];
  std::snprintf(fname_buf, sizeof(fname_buf), "%s.%05d", fROOTFilename.c_str(), nfile);
  std::string fname = fname_buf;

  if (nfile == 0) {
    fROOTFile = new TFile(fname.c_str(), "recreate", "", 0);
    if (!fROOTFile->IsOpen()) {
      WARNING("%s histogram file can't be opened", fname.c_str());
      return false;
    }
    INFO("%s histogram file is opened", fname.c_str());
  }
  else {
    TFile * oldfile = fROOTFile;
    TFile * newfile = new TFile(fname.c_str(), "recreate", "", 0);
    if (!newfile->IsOpen()) {
      WARNING("%s histogram file can't be opened", fname.c_str());
      return false;
    }

    TObject * obj;
    while ((obj = oldfile->GetList()->First())) {
      oldfile->Remove(obj);

      if (obj->InheritsFrom(TTree::Class())) {
        auto * t = static_cast<TTree *>(obj);
        t->SetDirectory(newfile);

        TIter nextb(t->GetListOfBranches());
        TBranch * branch = nullptr;
        while ((branch = static_cast<TBranch *>(nextb()))) {
          branch->SetFile(newfile);
        }

        if (t->GetBranchRef()) { t->GetBranchRef()->SetFile(newfile); }
      }
      else {
        if (newfile) newfile->Append(obj);
      }
    }

    delete oldfile;
    fROOTFile = newfile;
  }

  fROOTFileList.push_back(fname);
  return true;
}

void AbsHistogramer::Close()
{
  if (fROOTFile) {
    fROOTFile->Close();
    delete fROOTFile;
    fROOTFile = nullptr;
  }
}

void AbsHistogramer::Update()
{
  if (!fROOTFile) return;

  fROOTFile->cd();

  int nobj = fHistProxy->GetEntries();
  for (int i = 0; i < nobj; i++) {
    TObject * obj = fHistProxy->At(i);
    obj->Write(nullptr, TObject::kOverwrite);
  }

  fROOTFile->SaveSelf();
}