# CUPDAQ — Data Acquisition Software for CUP (Center for Underground Physics)

## Requirements
- **ROOT** ≥ 6.00  
- **libusb-1.0**  
- **yaml-cpp** for parsing config file
```bash
   dnf install yaml-cpp yaml-cpp-devel
- **(Optional)** **HDF5** — tested with version **1.14.1**

---

## Prerequisite

To allow CUPDAQ to access connected USB devices without root privileges:

1. Create a new file at  
   `/etc/udev/rules.d/88-notice.rules`

2. Copy the following lines into the file:

   ```text
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1000", MODE="0666"
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1501", MODE="0666"
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1502", MODE="0666"
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1503", MODE="0666"
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1903", MODE="0666"
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="2010", MODE="0666"

3. Reload udev rules with root privileges:
   
   ```bash
     sudo udevadm control --reload-rules && sudo udevadm trigger

## Build and Installation

   ```bash
     git clone https://github.com/cup-software2018/CUPDAQ.git
     cd CUPDAQ
     cmake -S . -B build
     cmake --build build -j
     cmake --install build --prefix [installation_path]

