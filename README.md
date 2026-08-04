# Stick-reaction-game-test-using-Arduino-

| Servo | Signal wire → Arduino |
|---|---|
| Servo 1 | D3 |
| Servo 2 | D5 |
| Servo 3 | D6 |
| Servo 4 | D9 |
| Servo 5 | D10 |

All 5 servo **V+ (red)** wires → external 5V supply positive rail
All 5 servo **GND (black/brown)** wires → external supply GND, **and** that GND must also tie to Arduino GND (common ground is required)

**Green Start Button:**
- One leg → Arduino **D2**
- Other leg → **GND**
- (Code uses `INPUT_PULLUP`, so no external resistor needed)

**Green LED** (skip if your green button already has a built-in LED — in that case just wire that LED's leads directly to D4/GND instead):
- Anode (+) → 220Ω resistor → **D4**
- Cathode (–) → **GND**

**Red Restart Button:**
- One leg → Arduino **D7**
- Other leg → **GND**

**Red LED** (same note as above if it's an illuminated button):
- Anode (+) → 220Ω resistor → **D8**
- Cathode (–) → **GND**

**Buzzer (passive buzzer recommended, for tone() to work properly):**
- + → **D11**
- – → **GND**

###
