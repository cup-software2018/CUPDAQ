

# CUPDAQ — Data Acquisition Software for CUP

CUPDAQ is a C++ based data acquisition system designed for the **Center for Underground Physics (CUP)**. It supports various digitizers and TCB (Trigger Control Board) modules used in neutrino and dark matter search experiments.

## System Requirements

CUPDAQ requires **C++17** and **CMake 3.16** or later. The following minimum OS/compiler versions are supported:

| Platform | Minimum Version | Default Compiler |
|----------|----------------|------------------|
| RHEL / Rocky / Alma | **8** | GCC 8+ |
| Fedora | **28** | GCC 8+ |
| Ubuntu / Debian | **20.04 / Buster** | GCC 9+ |

> **RHEL/CentOS 7 is not supported** — the default GCC (4.8.5) and CMake (2.8) are both too old.
> Using `devtoolset` (SCL) may work but is not tested or recommended.

---

## Requirements

| Dependency | Notes |
|------------|-------|
| **ROOT** >= 6.00 | Core, RIO, RHTTP components required |
| **CUPDataModel** >= v1.4 | Raw data object library — must be installed before CUPDAQ (see below); v1.4 adds `evtnum` to the HDF5 `EventInfo_t` |
| **libusb-1.0** | USB communication for Notice Korea boards |
| **yaml-cpp** | Configuration file parsing |
| **ZeroMQ / cppzmq** | Message server and network communication |
| **HDF5** >= 1.14 *(optional)* | Required only if CUPDataModel was built with HDF5 support |

> **Auto-installation (root only):** When CMake is invoked as `root`, missing system packages
> (ROOT, libusb, yaml-cpp, ZeroMQ) are installed automatically via the system package manager
> (`dnf`/`yum` on RHEL-family, `apt` on Debian/Ubuntu). This is controlled by the CMake option
> `CUPDAQ_AUTO_INSTALL_DEPENDENCIES` (default: `ON`).
> CUPDataModel cannot be auto-installed and must always be set up manually first.

### Manual package installation

If building without root privileges, install dependencies beforehand:

**RHEL / Rocky / Fedora**
```bash
sudo dnf install libusb1-devel yaml-cpp-devel zeromq-devel cppzmq-devel pkgconf-pkg-config
```

**Ubuntu / Debian**
```bash
sudo apt install libusb-1.0-0-dev libyaml-cpp-dev libzmq3-dev pkg-config
```

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

### Step 1 — Install CUPDataModel

CUPDataModel provides the raw data object libraries (RawObjs, HDF5Utils) used by CUPDAQ for both ROOT-format and HDF5-format raw data. It must be installed first.

```bash
git clone https://github.com/cup-software2018/CUPDataModel.git
cd CUPDataModel
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=./install
cmake --build build -j$(nproc)
cmake --install build
source install/setup_cupdm.sh
```

> When running as root, CUPDataModel will also auto-install its own missing dependencies (ROOT, HDF5).

### Step 2 — Build and Install CUPDAQ

```bash
git clone https://github.com/cup-software2018/CUPDAQ.git
cd CUPDAQ
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=./install
cmake --build build -j$(nproc)
cmake --install build
```

During the `cmake -S . -B build` configure step, if running as root, any missing system packages
(ROOT, libusb, yaml-cpp, ZeroMQ) are installed automatically before the build begins.

To **disable** auto-installation:
```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=./install -DCUPDAQ_AUTO_INSTALL_DEPENDENCIES=OFF
```

> **Non-standard CUPDataModel path:** If CUPDataModel was installed to a custom prefix, point CMake to it:
> ```bash
> cmake -S . -B build \
>   -DCMAKE_INSTALL_PREFIX=./install \
>   -DCUPDataModel_DIR=/path/to/cupdatamodel/install/lib64/cmake/CUPDataModel
> ```

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
* **OnlObjs**: ROOT object definitions for online data management and monitoring.
* **DAQTrigger**: Provides the software trigger interface (`AbsSoftTrigger`). Users can add custom trigger logic by subclassing `AbsSoftTrigger`.
* **OnlHistogramer**: Provides real-time event monitoring and histogramming capabilities.
* **DAQ**: The primary module containing the central management logic (`CupDAQManager`), executable scripts, and tools.

> **RawObjs** and **HDF5Utils** are now maintained in the [CUPDataModel](https://github.com/cup-software2018/CUPDataModel) package — an independent library for offline raw data analysis. CUPDAQ depends on it as an external package.

---

## Using CUPDAQ in Another CMake Project

Once CUPDAQ is installed, it can be consumed by downstream projects via `find_package`:

```cmake
find_package(CUPDataModel REQUIRED)
find_package(CUPDAQ REQUIRED)

target_link_libraries(MyApp PRIVATE CUPDAQ::DAQ)
```

All CUPDAQ modules are exported under the `CUPDAQ::` namespace. The primary entry point is `CUPDAQ::DAQ`, which transitively brings in all required modules and dependencies.

If CUPDAQ was installed to a non-standard path:
```bash
cmake -S . -B build -DCUPDAQ_DIR=/path/to/cupdaq/install/lib64/cmake/CUPDAQ
```

---

## Software Trigger

CUPDAQ supports user-defined software triggers applied per event during data acquisition. The trigger decision runs inside the build thread and can reject events before they are written to disk.

### Interface

All software triggers inherit from `AbsSoftTrigger` (`DAQTrigger/AbsSoftTrigger.hh`) and must implement three methods:

| Method | Description |
|--------|-------------|
| `DoConfig(AbsConfList *)` | Called before the run. Read trigger parameters from the config list. Call `SetEnable()` here to activate the trigger. |
| `InitTrigger()` | Called once just before data taking starts. Initialize internal state. |
| `DoTrigger(BuiltEvent *)` | Called per event. Return `true` to accept, `false` to reject. |

### Writing a Custom Trigger

1. Subclass `AbsSoftTrigger` and implement the three methods:

```cpp
// MyTrigger.hh
#include "DAQTrigger/AbsSoftTrigger.hh"

class MyTrigger : public AbsSoftTrigger {
public:
  MyTrigger() : AbsSoftTrigger("MyTrigger") {}

  void DoConfig(AbsConfList * configs) override;
  void InitTrigger() override;
  bool DoTrigger(BuiltEvent * event) override;

private:
  double fThreshold{0.0};
};
```

```cpp
// MyTrigger.cc
#include <yaml-cpp/yaml.h>
#include "MyTrigger.hh"

void MyTrigger::DoConfig(AbsConfList * configs)
{
  YAML::Node node = configs->GetYAMLNode("MyTrigger");
  if (!node) return;
  fThreshold = node["Threshold"].as<double>();
  if (node["Enabled"].as<int>()) 
      SetEnable();
}

void MyTrigger::InitTrigger() {}

bool MyTrigger::DoTrigger(BuiltEvent * event)
{
  fTotalInputEvent++;
  // ... trigger logic using fThreshold ...
  fNTriggeredEvent++;
  return true;
}
```

2. Add the trigger parameters to your YAML config file:

```yaml
MyTrigger:
  Enabled: 1
  Threshold: 100.0
```

3. Register it in the DAQ executable (e.g. `daq.cc`) before `DAQ->Run()`:

```cpp
auto * swtrigger = new MyTrigger();
DAQ->SetSoftTrigger(swtrigger);
```

### Trigger Report

At the end of each run, `PrintReport()` is called automatically and prints a summary:

```
========= SoftTrigger Report [MyTrigger] =========
              total input: 10000
                triggered: 4231
               efficiency: 42.31%
=====================================================
```

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