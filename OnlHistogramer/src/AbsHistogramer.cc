#include "TTree.h"
#include "TBranch.h"
#include "TBranchRef.h"

#include "OnlHistogramer/AbsHistogramer.hh"

AbsHistogramer::AbsHistogramer()
{
  fLog = ELogger::Instance();
  fHistProxy = new HistProxy();
}

AbsHistogramer::~AbsHistogramer()
{
  delete fHistProxy;
}

bool AbsHistogramer::Open()
{
  int nfile = fROOTFileList.size();
  const char * fname = Form("%s.%05d", fROOTFilename.Data(), nfile);

  if (nfile == 0) {
    fROOTFile = new TFile(fname, "recreate", "", 0);
    if (!fROOTFile->IsOpen()) {
      fLog->Warning("AbsHistogramer::Open", "%s histogram file can\'t be opened", fname);
      return false;
    }
    fLog->Info("AbsHistogramer::Open", "%s histogram file is opened", fname);
  }
  else {
    TFile * oldfile = fROOTFile;
    TFile * newfile = new TFile(fname, "recreate", "", 0);
    if (!newfile->IsOpen()) {
      fLog->Warning("AbsHistogramer::Open", "%s histogram file can\'t be opened", fname);
      return false;
    }

    TBranch * branch = nullptr;
    TObject * obj = oldfile->GetList()->First();

    while (obj) {
      oldfile->Remove(obj);

      if (obj->InheritsFrom(TTree::Class())) {
        auto * t = (TTree *)obj;
        t->SetDirectory(newfile);

        TIter nextb(t->GetListOfBranches());
        while ((branch = (TBranch *)nextb())) {
          branch->SetFile(newfile);
        }

        if (t->GetBranchRef()) {
          t->GetBranchRef()->SetFile(newfile);
        }
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

