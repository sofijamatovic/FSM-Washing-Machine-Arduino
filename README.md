# FSM-Washing-Machine-Arduino
# 🧼 Washing Machine Simulation (FSM & Arduino)

This project is a practical implementation and simulation of a washing machine control system based on the **Finite State Machine (FSM)** concept. It was developed as part of a university course project to demonstrate industrial process automation.

## 🚀 Run the Interactive Simulation in Your Browser!
No installation or hardware required. You can test the entire project live with a single click:

👉 **[CLICK HERE TO LAUNCH THE WOKWI SIMULATION]([PASTE_YOUR_WOKWI_LINK_HERE](https://wokwi.com/projects/469451626399185921))** 👈
---

## 🛠️ Hardware & Components Used
The virtual hardware circuit consists of the following components:
- **Arduino Mega 2560** microcontroller
- **I2C LCD 16x2 Display** (displays the current state and countdown timers in real-time)
- **3 Push Buttons** with software debouncing logic:
  - **Green Button:** Start / Reset
  - **Red Button:** Door sensor (simulates door lock/unlock)
  - **Blue Button:** Water level sensor (simulates drum full of water)
- **2 LED Indicators** representing physical actuators:
  - 🔴 **Red LED:** Water Valve / Pump (active during filling and draining phases)
  - 🔵 **Blue LED:** Drive Motor (active during washing, rinsing, and spinning phases)

---

## 🎮 How to Test the Project (Step-by-Step Guide)

When the simulation starts, the machine is in the **IDLE** (`MIROVANJE`) state. Follow these steps to complete a full wash cycle:

1. **Power On:** Click the **Green Button** (Start). The display changes to `ZAKLJ. VRATA` (Locking Door).
2. **Lock the Door:** Click the **Red Button** (Door). The state advances to `PUNJENJE VODE` (Filling Water) and the **Red LED** turns on (representing the water pump/valve).
3. **Trigger Water Sensor:** Once the water level is sufficient, click the **Blue Button** (Water).
4. **Fully Automated Cycle:** From this point, the program takes complete control. You do not need to click any more buttons. Watch the display and LEDs transition automatically:
   - **Washing (PRANJE):** (5 seconds, Blue LED active - motor spinning)
   - **Draining (ISPUSTANJE VODE):** (3 seconds, Red LED active - pump running)
   - **Rinsing (ISPIRANJE):** (4 seconds, Blue LED active - repeated for 2 cycles)
   - **Spin-drying (CENTRIFUGA):** (4 seconds, Blue LED active - high-speed spinning)
   - **End (KRAJ PROGRAMA):** LEDs turn off and a success message is displayed.

⚠️ **Safety Feature:** If you click the **Red Button** (opening the door) while the machine is actively washing, rinsing, or spinning, the system will instantly halt all processes, turn off all LEDs, and display **`GRESKA! VRATA!`** (Door Error). You can reset the system back to the IDLE state by clicking the **Green Button**.

---

## 📂 Repository Structure
- `fsm_washing_machine.c` – Pure C implementation containing the core state machine logic, structure definitions, and state transitions, entirely independent of any hardware.
- `arduino_washing_machine.ino` – Arduino sketch integrated with hardware pins, featuring soft debouncing, display rendering via I2C, and actuator controls.
- `diagram.json` – Wokwi circuit schema configuration file.
