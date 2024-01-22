# Voltage Regulator for Alternator Denso 120A

This repository implements the control of the alternator exciter in such a way as to achieve a voltage of 42V. The alternator winding is appropriately adapted to such a voltage.
More information in the [docs](/docs/).

This is test software.!

---
## YOUTUBE:
- [Test Alternator](https://youtu.be/Kwqm5Uq0JIM?si=3kcveeOb6-9aURg2).

---

## Connection diagram:

-[Diagram Alternator](/docs/Diagram.png).

-[Diagram Regulator](/docs/Diagram_Regulator.png).

---
## Hardware
The original winding connection was a double star as shown in this [document](/docs/Diody.jpg) To increase the voltage, I had to disconnect all the windings and connect them through Gretz bridges.
I intend to use esp32 to control the alternator voltage, and N-type mosfets to control the magnetic field of the exciter via PWM

---
## My Project
I need to create a battery charger for howerboards that I use to build an autonomous lawn mower. The alternator will be driven from the shaft of the petrol lawnmower. It has about 3000 rpm. It should be enought.

---
