R__LOAD_LIBRARY(libHDF5Utils)

void write_fadc_event(const char * filename = "test_fadc_events.h5", int n_events = 100)
{
  std::cout << "========== [GENERATE] Creating FADC Display Test Data ==========\n";

  const int ndp = 256;
  const int n_channels = 4; // Exactly 4 channels for the 2x2 display

  // Setup DataWriter
  H5DataWriter writer(filename);
  H5FADCEvent * wEvent = new H5FADCEvent();

  wEvent->SetNDP(ndp);
  wEvent->SetBufferCapacity(50); // Flush every 50 events

  writer.SetData(wEvent);
  writer.SetSubrun(1);
  writer.Open();

  std::cout << "Generating " << n_events << " events with 4 channels each...\n";

  for (int i = 0; i < n_events; ++i) {
    EventInfo_t info;
    info.tnum = i;
    info.ttime = static_cast<std::uint64_t>(i) * 1000ULL;

    std::vector<FChannel_t> channels(n_channels);

    for (int ch = 0; ch < n_channels; ++ch) {
      channels[ch].id = ch;
      channels[ch].tbit = 1;
      channels[ch].ped = 200;
      channels[ch].time = info.ttime + ch;

      // Create a deterministic mock pulse shape
      // Baseline is around 200. Pulse starts at different positions for each channel.
      int pulse_start = 30 + (ch * 20) + (i % 10);
      double amplitude = 500.0 + (ch * 100.0);
      double decay_constant = 15.0 + ch;

      for (int s = 0; s < ndp; ++s) {
        double signal = 200.0; // Baseline

        // Add a simple exponential decay pulse if we are past the start index
        if (s >= pulse_start) {
          signal += amplitude * std::exp(-static_cast<double>(s - pulse_start) / decay_constant);
        }

        channels[ch].waveform[s] = static_cast<std::uint16_t>(signal);
      }
    }

    wEvent->AppendEvent(info, channels);
  }

  writer.PrintStats();
  writer.Close();
  delete wEvent;

  std::cout << "Data generation complete. Ready for display macro.\n";
  std::cout << "==============================================================\n";
}