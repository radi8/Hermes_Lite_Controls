# Hermes_Lite_Controls
Arduino software to provide signaling and filter switching for a Hermes-Lite

Still very experimental but works with my HiQSDR which I am using as a test bed because I have opto isolated control lines so the gear is safe and it works with Quisk which I am using for Hermes-Lite. I was messing around on the bench and added the pullups because open inputs gave variable readings. When they are connected to the radio the lines are either pulled down or up so the unstable readings don't happen.

The Arduino could actually connect to fpga via I2C and all the band change signals and ptt etc. be sent over that. In addition user data from rotator, power and swr could be fed back to the radio. Steve would of course come into the scene here and the compatibility with Hermes needs consideration but I can't see that he physical side of passing signals would be an issue but the protocol would be which we don't alter.

A serial connection could also be made to the fpga using the RS232 serial output of Arduino. Wouldn't use this for time critical data but good for sending stuff like power and swr back and forth.achieved

Vih (Voltage Input High) for Arduino is 3.0 Volts. Therefore switching will be unreliable for voltages less than this. We are sailing close to the wind driving Arduino inputs direct from the fpga with very little headroom at nearly 3.3 volts and again Dieter's isolation board will achieve level translation in a safe manner.

The delays for the Tx/Rx relay, PA Bias and filter switching are going to be set in the Arduino software. Rather than hard coding them, they could be set up as a config parameter in the radio software e.g.Quisk and passed as a parameter through the firmware via I2C or serial to the Arduino. Otherwise a small utility could be written in Qt, C or Python to program the Arduino's EEprom via USB for these timings. The same applies to a lookup/translation table so any input can produce any output condition.
