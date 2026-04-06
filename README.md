

# CUPDAQ — Data Acquisition Software for CUP

CUPDAQ is a C++ based data acquisition system designed for the **Center for Underground Physics (CUP)**. It supports various digitizers and TCB (Trigger Control Board) modules used in neutrino and dark matter search experiments.

## Requirements

* **ROOT** >= 6.00 (Core, RIO, Hist, Gpad)
* **libusb-1.0** (USB communication for Notice Korea boards)
* **yaml-cpp** (Configuration file parsing)
  * Command: `sudo dnf install yaml-cpp yaml-cpp-devel`
* **ZeroMQ & cppzmq** (Message server and network communication)
  * Command: `sudo dnf install zeromq zeromq-devel cppzmq-devel`
* **(Optional) HDF5** (High-performance data writing, tested with 1.14.2)

---

## Prerequisite: USB Access (udev rules)

To allow CUPDAQ to access connected USB devices without root privileges, configure the udev rules as follows:

1. **Create a new rule file**
   `sudo vi /etc/udev/rules.d/88-notice.rules`

2. **Copy the following lines into the file**
   ```text
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1000", MODE="0666"
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1501", MODE="0666"
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1502", MODE="0666"
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1503", MODE="0666"
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="1903", MODE="0666"
   SUBSYSTEM=="usb", ATTR{idVendor}=="0547", ATTR{idProduct}=="2010", MODE="0666"
   ```

3. **Reload the udev rules**
   `sudo udevadm control --reload-rules && sudo udevadm trigger`

---

## Build and Installation

CUPDAQ uses a modern CMake build system that supports **relocatable** installations via RPATH.

   ```bash
   # Clone the repository
   git clone https://github.com/cup-software2018/CUPDAQ.git
   cd CUPDAQ

   # Configure the build
   cmake -S . -B build -DCMAKE_INSTALL_PREFIX=./install

   # Build using all available cores
   cmake --build build -j$(nproc)

   # Install to the prefix directory
   cmake --install build
   ```

## Environment Setup

After installation, you must source the environment setup script. This script is **relocatable**; if you move the installation directory to another location or server, simply source the script from the new path.

   ```bash
   cd [your_installation_path]
   source setup_cupdaq.sh
   ```

### What this script does:
* Sets **CUPDAQ_DIR** to the current installation root.
* Updates **PATH** to include DAQ binaries (e.g., `daq`, `tcb`, `merger`).
* Updates **LD_LIBRARY_PATH** for CUPDAQ shared libraries.
* Updates **ROOT_INCLUDE_PATH** to ensure ROOT can find CUPDAQ classes at runtime.

---

## Directory Structure

Upon successful installation, the directory will look like this:

   ```text
   install/
   ├── bin/                # Executables (daq, tcb, test_utils...)
   ├── include/            # Header files for all modules
   ├── lib64/              # Shared libraries (.so) and ROOT dicts (.pcm)
   └── setup_cupdaq.sh     # Environment initialization script
   ```

---

## Module Overview

The CUPDAQ system is organized into several modular components to maintain clean separation of concerns and facilitate scalable development:

* **OnlConsts**: Header-only library containing fundamental constants used across the DAQ system.
* **DAQUtils**: Header-only utility library providing common helper functions and tools.
* **DAQConfig**: Handles the parsing and management of YAML configuration files for DAQ parameters.
* **Notice**: Hardware communication library handling USB interfacing with Notice Korea boards.
* **DAQSystem**: Core system-level classes and logic for managing hardware components and states.
* **OnlObjs / RawObjs**: Object definitions (ROOT objects and standalone) for raw data packaging and online data management.
* **DAQTrigger**: Handles the trigger configurations, logic, and synchronizations.
* **OnlHistogramer**: Provides real-time event monitoring and histogramming capabilities.
* **HDF5Utils**: (Optional) Integrated tools for high-performance data writing in the HDF5 format.
* **DAQ**: The primary module containing the central management logic (`CupDAQManager`), executable scripts, and tools.

---

## Customizing the Execution Script (executedaq.sh)

The `executedaq.sh` script is responsible for setting up the runtime environment and launching the DAQ binaries. Depending on your system infrastructure, you may need to modify the environment loading section.

### Modifying Base Environment
If your system requires pre-loading specific modules (e.g., ROOT, compiler, or experiment-wide libraries), locate the following section in `bin/executedaq.sh`:

```bash
# -----------------------------------------------------------------------------
# 1. External Infrastructure Setup (ROOT, python, etc.)
# -----------------------------------------------------------------------------
# [IMPORTANT] Modify this section to source your system's environment 
# e.g., source /path/to/your/root/bin/thisroot.sh
# -----------------------------------------------------------------------------

# Example: Default IBS CUP setup
if [ -f ~cupsoft/prod_setup.sh ]; then
    source ~cupsoft/prod_setup.sh 3.0
fi
```