# 🏎️ Peacock Elettrica - Steering Wheel Dashboard Firmware

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white) ![STM32](https://img.shields.io/badge/STM32-03234B?style=for-the-badge&logo=stmicroelectronics&logoColor=white) ![CAN Bus](https://img.shields.io/badge/CAN_Bus-FFB900?style=for-the-badge&logo=data-line&logoColor=black) ![Nextion](https://img.shields.io/badge/Nextion_HMI-1A9C3E?style=for-the-badge&logoColor=white)

> 🌐 **Language / Lingua:** 🇬🇧 **English**
> 
> 📖 Firmware and UI assets for the steering wheel display of the **Peacock Elettrica** Formula Student race car, developed by **[Polimarche Racing Team](https://www.polimarcheracingteam.com/it/)**.

---

This repository contains the bare-metal C firmware and the graphical assets for the custom steering wheel dashboard. The system acquires high-speed telemetry from the vehicle's CAN bus and drives a graphical HMI display in real-time, providing the driver with critical dynamic parameters, active vehicle states, and safety diagnostics.

**Hardware Stack:**
* **MCU:** STM32 Nucleo-F303K8T6 (ARM Cortex-M4)
* **Display:** Nextion NX4832T035 (3.5" HMI)
* **Transceiver:** MCP2551 (CAN Bus)

---

## 📌 Overview & Architecture

The firmware is designed for ultra-low latency and absolute stability during race conditions. It implements a custom scheduler handling specific tasks (Input reading, CAN transmission, Display updating) and relies heavily on hardware-level optimizations:

* **Non-Blocking UART via DMA:** Display commands are buffered and transmitted via Direct Memory Access, freeing the CPU for continuous CAN parsing.
* **Graphics Edge-Detection:** To prevent UART bottlenecking, the display task implements state-tracking. UI elements (colors, icons, backgrounds) are only transmitted when a state change occurs, not at every cycle.
* **Safety Latching:** Critical errors are latched on screen for a minimum safety time (2000ms) to ensure visibility even during sensor fluctuations.

---

## 📊 Parameters Displayed

The dashboard UI dynamically adapts to the car's state.

* 🏎️ **Vehicle Dynamics:** Real-time Car Speed, Wheel Speeds, and dual Steering Angle Sensor (SAS) progress bars.
* 🔋 **Powertrain:** State of Charge (SoC) progress bar, LV Battery Voltage, and Max Discharge/Regen currents.
* ♨️ **Thermals:** Average and Max temperatures for Inverters, Motors, Battery Pack, and Coolant. *Values turn red if they exceed safety thresholds.*
* ⚙️ **Engine Map (E-MAP):** Dynamic UI themes based on the active map:
    * 🟢 **ECO** (Green Theme)
    * 🔵 **NORMAL** (Blue Theme)
    * 🔴 **GAS** (Red Theme)
* ⚠️ **Diagnostics:** Real-time error matrix parsing. Displays specific proprietary Inveter Fault codes (e.g., `L2319`) or internal BMS/PDM error IDs.
* ✅ **State Machines:** R2D (Ready to Drive) and SDC (Shutdown Circuit) visual status indicators.

---

## 🖼️ Dashboard Preview

The UI relies on a `crop_image` technique to ensure seamless transitions between Engine Maps without reloading entire graphical pages, saving processing power on the Nextion internal chip.

<p align="center">
  <img src="dashboard-layout/render/full_layout_fhd.png" width="1000" alt="Dashboard Layouts and Map Comparisons"/>
</p>

*The image shows the different graphic layouts and color schemes adapting to the selected Engine Map (ECO, NORM, GAS).*

---

## 🗂️ Project Structure

The repository is organized to separate UI assets from the embedded logic:

```text
dashboard-peacockElettrica/
├── README.md
├── elements_display.txt                # Nextion variable mapping reference
├── error_codes.txt                     # Diagnostic error ID reference
│
├── dashboard-layout/                   # Nextion Editor UI Assets
│   └── font/                           # Custom generated Michroma fonts
│
├── documents/                          # Project specifications and manuals
│   ├── Specifica_Pulsanti_Volante.docx
│   └── Dashboard_DISPLAY_v2.docx
│
└── dashboard-stm32/                    # STM32CubeIDE Project
    ├── dashboard-peacockElettrica.ioc  # CubeMX Configuration
    ├── STM32F303K8TX_FLASH.ld          # Linker Script
    └── Core/
        ├── Inc/                        # Headers (.h)
        │   ├── car_data.h              # Main telemetry struct & Error Enums
        │   ├── Scheduler.h
        │   ├── Tasks.h
        │   └── Communication/
        │       ├── can.h
        │       └── serial.h
        └── Src/                        # Source Code (.c)
            ├── main.c
            ├── Scheduler.c
            ├── shift_register.c        # Custom bit-banging shift register logic
            ├── Tasks/                  # Application Logic
            │   ├── task_can.c
            │   ├── task_display.c      # Dynamic UI rendering logic
            │   ├── task_errors.c
            │   └── task_inputs.c
            └── Communication/
                ├── can.c               # CAN Filter and RX/TX logic
                └── serial.c            # DMA UART Nextion wrappers

```

## 🔄 References & Resources
- [Nextion Display](https://nextion.tech/) - Free Dedicated Software for Nextion HMI Display
- [Magnific (formerly Freepik.com)](https://www.magnific.com/it) - All icons used in the web dashboard
- [Shields.io](https://shields.io/) - for aesthetic elements for the repository (using [SimpleIcons](https://simpleicons.org))

---
## 📄 License

This project is licensed under the **[Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)** license.

Under these terms, you are free to copy and redistribute the material in any medium or format, as long as you comply with the following conditions:
* **Attribution (BY):** You must give appropriate credit to the original author (Polimarche Racing Team) and provide a link to the license.
* **NonCommercial (NC):** You may not use the material for commercial purposes.
* **NoDerivatives (ND):** If you remix, transform, or build upon the material, you may not distribute the modified material.

For the full legal code, please read the `LICENSE` file in this repository or visit the official Creative Commons website.

---

## 📬 Contact

For questions or collaboration, contact:

- 📧 `zingaale@gmail.com`
