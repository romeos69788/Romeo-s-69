# ESP32 38 pin — HP CONTROL BOARD v1.0

**Πλακέτα:** Romeos HP Board (water-to-water heat pump)  
**Module:** ESP32 DevKit 38 pin (ESP-32D)  
**USB:** προς **εξωτερική άκρη** PCB — πρόσβαση καλωδίου χωρίς αποσυναρμολόγηση.

**H2** = αριστερή στήλη · pin 1 πάνω = **3V3** · pin 19 κάτω = **5V**  
**H1** = δεξιά στήλη · pin 1 πάνω = **GND** · pin 7 = **GND**

*Κλειδωμένο 2026-06-13 — μόνο HP board.*

---

## H2 — pins 1…19 (σειρά 3V3 → 5V)

| Pin | Silk (DevKit) | Net HP | GPIO | Σημείωση |
|-----|---------------|--------|------|----------|
| **1** | 3V3 | `3V3` | — | Τροφοδοσία module |
| **2** | EN | — | EN | **NC** (pull-up onboard) |
| **3** | VP | `OEM_RUN_IN` | 36 | Opto είσοδος RUN από OEM |
| **4** | VN | `OEM_FAULT_IN` | 39 | Opto είσοδος FAULT από OEM |
| **5** | IO34 | `FLOW_SIG` | 34 | Αισθητήρας ροής (input only) |
| **6** | IO35 | `PRESS_DISCH` | 35 | Πίεση εξαγωγής — φάση 2 (input only) |
| **7** | IO32 | `RS485_DE` | 32 | MAX485 DE+RE |
| **8** | IO33 | `LED_STATUS` | 33 | LED κατάστασης |
| **9** | IO25 | `REL_HEAT_CALL` | 25 | Ρελέ κλήσης θέρμανσης → OEM |
| **10** | IO26 | `REL_PUMP` | 26 | Ρελέ κυκλοφορητή HP |
| **11** | IO27 | `REL_COOL_REV` | 27 | Ρελέ COOL / reversing — standby |
| **12** | IO14 | `DEFROST_SIG` | 14 | Σήμα defrost (CN2) |
| **13** | IO12 | — | 12 | **NC** (strap — μην συνδέσεις) |
| **14** | GND | `GND` | — | Γείωση |
| **15** | IO13 | `POT_CS` | 13 | CS → MCP41050 (thermistor spoof) |
| **16** | D2 | — | flash | **NC** |
| **17** | D3 | — | flash | **NC** |
| **18** | CMD | — | flash | **NC** |
| **19** | 5V | `5V_ESP` | — | VIN module (κλάδος F3) |

---

## H1 — pins 1…19 (σειρά GND / SPI / comm)

| Pin | Silk (DevKit) | Net HP | GPIO | Σημείωση |
|-----|---------------|--------|------|----------|
| **1** | GND | `GND` | — | Γείωση |
| **2** | IO23 | `SPI_MOSI` | 23 | MCP41050 |
| **3** | IO22 | — | 22 | **NC** |
| **4** | TX | `UART0_TX` | 1 | Serial debug / USB |
| **5** | RX | `UART0_RX` | 3 | Serial debug / USB |
| **6** | IO21 | — | 21 | **NC** |
| **7** | GND | `GND` | — | Γείωση |
| **8** | IO19 | `SPI_MISO` | 19 | MCP41050 |
| **9** | IO18 | `SPI_SCK` | 18 | MCP41050 |
| **10** | IO5 | `SERVO_PWM` | 5 | Servo 3-way mixing valve |
| **11** | IO17 | `RS485_TX` | 17 | UART2 → MAX485 |
| **12** | IO16 | `RS485_RX` | 16 | UART2 → MAX485 |
| **13** | IO4 | `DS18_DATA` | 4 | 1-Wire → CN4 (DS18B20) |
| **14** | IO0 | — | 0 | **NC** (boot strap) |
| **15** | IO2 | — | 2 | **NC** (strap) |
| **16** | IO15 | — | 15 | **NC** (strap) |
| **17** | D1 | — | flash | **NC** |
| **18** | D0 | — | flash | **NC** |
| **19** | CLK | — | flash | **NC** |

---

## Σύνοψη nets (για ERC / firmware)

**Είσοδοι:** `OEM_RUN_IN`, `OEM_FAULT_IN`, `FLOW_SIG`, `DEFROST_SIG`, `PRESS_DISCH` (φάση 2)  
**Έξοδοι:** `REL_HEAT_CALL`, `REL_PUMP`, `REL_COOL_REV`, `RS485_DE`, `LED_STATUS`, `SERVO_PWM`, `POT_CS`  
**Bus:** `SPI_MOSI`, `SPI_MISO`, `SPI_SCK`, `RS485_TX`, `RS485_RX`, `DS18_DATA`  
**Debug:** `UART0_TX`, `UART0_RX`  
**Τροφοδοσία:** `3V3`, `5V_ESP`, `GND`
