#include <iostream>

#include "DAQUtils/ELog.hh"
#include "Notice/NKMiniTCB.hh"

int main(int argc, char ** argv)
{
  if (argc != 2) {
    std::cerr << "Usage: test_minitcb <ip-address>" << std::endl;
    return 1;
  }

  std::string ip = argv[1];

  NKMiniTCB tcb;
  tcb.SetIPAddress(ip.c_str());

  INFO("Testing NKMiniTCB Open/Close with IP = %s", ip.c_str());

  int stat = tcb.Open();
  if (stat != 0) {
    ERROR("Failed to open NKMiniTCB (stat=%d)", stat);
    return 1;
  }

  INFO("NKMiniTCB successfully opened.");

  tcb.Close();
  INFO("NKMiniTCB closed.");

  return 0;
}
