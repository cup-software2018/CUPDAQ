R__LOAD_LIBRARY(libHDF5Utils)

void draw_fadc(const char * filename = "test_fadc_events.h5")
{
  std::cout << "========== [DISPLAY] FADC Waveform Viewer ==========\n";

  // 1. Setup DataReader
  H5DataReader reader;
  H5FADCEvent * rEvent = new H5FADCEvent();

  reader.Add(filename);
  reader.SetData(rEvent);

  if (!reader.Open()) {
    std::cerr << "Failed to open the file: " << filename << "\n";
    return;
  }

  int total_events = reader.GetEntries();
  std::cout << "Total events available: " << total_events << "\n\n";

  // 2. Setup ROOT Canvas and Graphs
  TCanvas * c1 = new TCanvas("c1", "FADC Waveform Display", 1200, 800);
  c1->Divide(2, 2); // 2x2 grid for 4 channels

  // Create 4 TGraph objects for the 4 expected channels
  TGraph * gr[4];
  for (int i = 0; i < 4; ++i) {
    gr[i] = new TGraph();
    gr[i]->SetLineColor(4); // Blue line
    gr[i]->SetLineWidth(2);
    gr[i]->SetMarkerStyle(20);
    gr[i]->SetMarkerSize(0.5);
    gr[i]->SetMarkerColor(2); // Red markers
  }

  // 3. Event Loop with User Input
  for (int i = 0; i < total_events; ++i) {
    if (rEvent->ReadEvent(i) < 0) {
      std::cerr << "Failed to read event " << i << "!\n";
      break;
    }

    EventInfo_t info = rEvent->GetEventInfo();
    FChannel_t * data = rEvent->GetData();
    int ndp = rEvent->GetNDP();

    // Determine how many hits to draw (safeguard up to 4)
    int hits_to_draw = (info.nhit < 4) ? info.nhit : 4;

    // Draw waveforms on the canvas
    for (int ch = 0; ch < 4; ++ch) {
      c1->cd(ch + 1); // Select pad (1 to 4)

      if (ch < hits_to_draw) {
        gr[ch]->Set(0); // Clear previous points

        // Fill the graph with waveform samples
        for (int s = 0; s < ndp; ++s) {
          gr[ch]->SetPoint(s, s, data[ch].waveform[s]);
        }

        // Set titles and axis labels
        TString title =
            Form("Event %u (TNum), Channel ID %u;Sample Index;ADC Value", info.tnum, data[ch].id);
        gr[ch]->SetTitle(title);

        // Draw with Axis and Line
        gr[ch]->Draw("APL");
      }
      else {
        // Clear the pad if there is no data for this channel
        c1->GetPad(ch + 1)->Clear();
      }
    }

    // Update the canvas to show the new plots
    c1->Update();

    // 4. Prompt for user action
    std::cout << "[Event " << i << "] Displayed " << hits_to_draw << " channels.\n";
    std::cout << "Press [Enter] for the next event, or type 'q' and [Enter] to quit: ";

    std::string input;
    std::getline(std::cin, input);

    if (input == "q" || input == "Q") {
      std::cout << "Exiting display loop...\n";
      break;
    }
  }

  // Cleanup
  reader.Close();
  delete rEvent;

  std::cout << "========== Display Session Finished ==========\n";
}