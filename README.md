# Display DASHBOARD 24-25
**`Nucleo F303K8 + Nextion NX4832T035 ⚡`**

This repository contains the firmware for the steering wheel display DASHBOARD using the STM32 Nucleo-F303K8 board and the Nextion NX4832T035 display. The firmware is designed to show in real time the current value of the most important parameters that the driver needs to keep under control.

## Parameters displayed
- 🏎  **Speed Value** : Current vehicle speed;
- 🔋  **State of Charge (SoC) value**: Real time battery pack charge percentage;
- ♨️ **Temperatures**: The temperatures shown are those of Engines, Inverter, Battery pack and Coolant;
- ⚙️ **Engine Map**: Numeric value of the engine map set to run;
- ⚠️ **Errors**: Numeric value of the error;
- ✅ **R2D**: if the car is ready to drive;

## Renders
Main page render:

<img src="DASHBOARD-STM32F303K8T6-V1\Layout-Dashboard/MAINV2.png" alt="Main Page" width="200">

- The background of temperatures slot will be `RED` if the respective value exceeds the maximum expected value
- The SoC BarValue changes its color based on the corresponding value (`RED`,`ORANGE`,`YELLOW`,`GREEN`)
- The `Fault Status` appears only in case a packet with an error ID is read from the `CAN-BUS` network

## Credits

This project draws inspiration from several sources and projects:

- **CAN Bus Communication**:  
   🎞️ [CAN Bus Tutorial](https://www.youtube.com/watch?v=KHNRftBa1Vc)
       [MCP2551 setup](https://www.youtube.com/watch?v=_Cntd87oce4)
- **NEXTION user guide**:  
   🎞️ [Arduino tutorial](https://www.youtube.com/watch?v=wIWxSLVAAQE)

## 👀 Interested in Learning More?

If you have any questions, would like to discuss this project further, or are interested in potential collaboration opportunities, please feel free to connect with me through the following channels:

- LinkedIn: [polimarcheracingteam](https://www.linkedin.com/company/polimarcheracingteam/posts/?feedView=all)
- Instagram: [polimarcheracingteam](https://www.instagram.com/polimarcheracingteam/)
- Email: [formulasae@sm.univpm.it](mailto:formulasae@sm.univpm.it)
