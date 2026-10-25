\# AeroGuard — Smart Avionics Equipment Health Monitoring



\## Project Overview



AeroGuard is an educational IoT and embedded-systems prototype designed to simulate equipment health monitoring using an ESP32-S3.



The system monitors temperature and humidity and uses a potentiometer as a simulated analog anomaly input. It classifies the system status into NORMAL, WARNING, and CRITICAL states.



\*\*Disclaimer:\*\* This is an educational simulation prototype. It is not a certified avionics system and must not be used for real aircraft safety decisions.



\## Objectives



\- Monitor environmental measurements.

\- Simulate abnormal analog input values.

\- Filter measurements using a moving average.

\- Classify the system into three health states.

\- Provide visual and audible alerts.

\- Display measurements and status on an LCD.

\- Send telemetry through the Serial Monitor.



\## Hardware Components



\- ESP32-S3 development board

\- DHT11 temperature and humidity sensor

\- Potentiometer for simulated analog anomalies

\- Green, yellow, and red LEDs

\- Buzzer

\- 16×2 I2C LCD



\## Software and Technologies



\- Arduino C++

\- ESP32-S3

\- Cirkit Designer simulation

\- DHT sensor library

\- LiquidCrystal\_I2C library



\## System States



| State | Description |

|---|---|

| NORMAL | Measurements are below warning thresholds. |

| WARNING | At least one measurement reaches a warning threshold. |

| CRITICAL | At least one measurement reaches a critical threshold or the DHT sensor reading fails. |



\## Data Processing



The firmware samples the sensors periodically and calculates a moving average over up to five readings. It evaluates the filtered values against configurable thresholds and updates the outputs without using delay-based loops.



Telemetry is printed periodically to the Serial Monitor.



\## Current Simulation Status



The initial simulation has produced serial telemetry with example readings:



\- Temperature: 25.0 °C

\- Humidity: 60.0%

\- Analog input: 1778

\- System state: NORMAL



WARNING and CRITICAL transitions, as well as all alert outputs, still need to be tested and documented separately.



\## Limitations



The potentiometer is only a simulated analog anomaly input. It does not measure real vibration or equipment damage. Thresholds are for educational testing and are not validated aviation limits.



\## Future Improvements



\- Test and document all three states.

\- Add a Python dashboard for telemetry visualization.

\- Store measurements for historical analysis.

\- Improve fault detection and test coverage.

\- Document the circuit and simulation results.



\## Author



IoT and Embedded Systems Student

