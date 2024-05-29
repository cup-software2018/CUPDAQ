#include "TH1D.h"

#include "DAQConfig/AbsConf.hh"
#include "OnlObjs/ADCHeader.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlHistogramer/FADCHistogramer.hh"

FADCHistogramer::FADCHistogramer() {}

FADCHistogramer::~FADCHistogramer() {}

void FADCHistogramer::Book() {}

void FADCHistogramer::Fill(BuiltEvent * event) {}
