#include "TH1D.h"

#include "DAQConfig/FADCTConf.hh"
#include "DAQConfig/IADCTConf.hh"

#include "OnlHistogramer/FADCHistogramer.hh"
#include "OnlObjs/FADCRawChannel.hh"
#include "OnlObjs/FADCRawEvent.hh"

void FADCHistogramer::Book()
{
  if (!fConfigList) return;

  int nadc = fConfigList->GetEntries();
  for (int i = 0; i < nadc; i++) {
    auto * conf = static_cast<AbsConf *>(fConfigList->At(i));
    ADC::TYPE type = conf->GetADCType();

    int ndp = 0;
    int nch = 0;
    int rl = 0;

    if (type == ADC::FADCS || type == ADC::FADCT) {
      auto * fadc_conf = static_cast<FADCTConf *>(conf);
      nch = fadc_conf->NCH();
      rl = fadc_conf->RL();
      ndp = 64 * rl - 16;
    }
    else if (type == ADC::GADCS || type == ADC::GADCT) {
      auto * fadc_conf = static_cast<FADCTConf *>(conf);
      nch = fadc_conf->NCH();
      rl = fadc_conf->RL();
      ndp = 16 * rl - 2;
    }
    else if (type == ADC::MADCS || type == ADC::MADCT) {
      auto * fadc_conf = static_cast<FADCTConf *>(conf);
      nch = fadc_conf->NCH();
      rl = fadc_conf->RL();
      ndp = 16 * rl - 16;
    }
    else if (type == ADC::IADCT) {
      auto * fadc_conf = static_cast<IADCTConf *>(conf);
      nch = fadc_conf->NCH();
      rl = fadc_conf->RL();
      if (rl > 0) ndp = 8 * rl - 2;
    }

    if (ndp <= 0) continue;

    int daqid = conf->GetDAQID();
    int mid = conf->MID();

    for (int j = 0; j < nch; j++) {
      TString name = Form("hWf_%d_%d_%d", daqid, mid, j);
      TString title = Form("Wf: DAQ %d SID %d CH %d", daqid, mid, j);

      TH1D * h = new TH1D(name, title, ndp, 0, ndp);
      h->SetXTitle("Samples");
      h->SetYTitle("ADC");
      fHistProxy->Add(h);
    }
  }
}

void FADCHistogramer::Fill(BuiltEvent * event)
{
  if (!event) return;

  int daqid = event->GetDAQID();
  int nadc = event->GetEntries();

  for (int i = 0; i < nadc; i++) {
    auto * adcevt = static_cast<AbsADCRaw *>(event->At(i));
    ADC::TYPE type = adcevt->GetADCType();

    auto * fadcevt = static_cast<FADCRawEvent *>(adcevt);
    int mid = fadcevt->GetADCHeader()->GetMID();
    int nch = fadcevt->GetNCH();

    for (int j = 0; j < nch; j++) {
      FADCRawChannel * ch = fadcevt->GetChannel(j);
      if (!ch) continue;

      TString name = Form("hWf_%d_%d_%d", daqid, mid, j);
      auto * h = static_cast<TH1D *>(fHistProxy->FindObject(name));

      if (h) {
        int ndp = ch->GetNDP();
        unsigned short * adc = ch->GetADC();
        h->Reset();
        for (int k = 0; k < ndp; k++) {
          h->SetBinContent(k + 1, adc[k]);
        }
      }
    }
  }
}
