#include "TBranch.h"
#include "TBranchRef.h"
#include "TTree.h"

#include "OnlHistogramer/AbsHistogramer.hh"

AbsHistogramer::AbsHistogramer() { fHistProxy = new HistProxy(); }

AbsHistogramer::~AbsHistogramer() { delete fHistProxy; }

bool AbsHistogramer::Open()
{
  int nfile = static_cast<int>(fROOTFileList.size());
  const char * fname = Form("%s.%05d", fROOTFilename.Data(), nfile);

  if (nfile == 0) {
    fROOTFile = new TFile(fname, "recreate", "", 0);
    if (!fROOTFile->IsOpen()) {
      WARNING("%s histogram file can't be opened", fname);
      return false;
    }
    INFO("%s histogram file is opened", fname);
  }
  else {
    TFile * oldfile = fROOTFile;
    TFile * newfile = new TFile(fname, "recreate", "", 0);
    if (!newfile->IsOpen()) {
      WARNING("%s histogram file can't be opened", fname);
      return false;
    }

    TBranch * branch = nullptr;
    TObject * obj = oldfile->GetList()->First();

    while (obj) {
      oldfile->Remove(obj);

      if (obj->InheritsFrom(TTree::Class())) {
        auto * t = static_cast<TTree *>(obj);
        t->SetDirectory(newfile);

        TIter nextb(t->GetListOfBranches());
        while ((branch = static_cast<TBranch *>(nextb()))) {
          branch->SetFile(newfile);
        }

        if (t->GetBranchRef()) { t->GetBranchRef()->SetFile(newfile); }
        continue;
      }

      if (newfile) newfile->Append(obj);
      oldfile->Remove(obj);
    }

    delete oldfile;
    oldfile = nullptr;
    oldfile = newfile;
  }

  fROOTFileList.push_back(fname);
  return true;
}

void AbsHistogramer::Close()
{
  fROOTFile->Close();
  delete fROOTFile;
  fROOTFile = nullptr;
}

void AbsHistogramer::Update()
{
  fROOTFile->cd();

  int nobj = fHistProxy->GetEntries();
  for (int i = 0; i < nobj; i++) {
    TObject * obj = fHistProxy->At(i);
    obj->Write(nullptr, TObject::kOverwrite);
  }

  fROOTFile->SaveSelf();
}
