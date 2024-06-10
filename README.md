# CUPDAQ - DAQ Software for CUP (Center for Underground Physics)

## Requirements
##### ROOT: > V6 
##### libusb-1.0 
##### (optional) HDF5: tested with 1.14.1

## Prerequisite

Copy below to /etc/udev/rules.d/88-notice.rules, run "`udevadm control --reload-rules && udevadm trigger`" with root privilege.
```
SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1000", MODE="0666" 
SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1501", MODE="0666" 
SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1502", MODE="0666" 
SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1503", MODE="0666" 
SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1903", MODE="0666"
SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="2010", MODE="0666"
```

## Build & Installation

```
>> git clone https://github.com/cup-software2018/CUPDAQ.git
>> mkdir build; cd build
>> cmake ../CUPDAQ -DCMAKE_INSTALL_PREFIX=[where to install cupdaq]
>> make -j[NCUP]; make install
```

## Usage
this is test