# S3 - The Simpler Stereo Sampler!

S3 is a very simple sampler VST plugin I wrote for making oscilloscope music easier with my workflow. There's no up/downsampling, no time/pitch stretching, and that means there's no distortions to the oscilloscope visuals! The only automations are the current sample slot to play from, and options to reset the current sample or all samples.

It's best to compile it yourself, especially on MacOS since I don't want to pay for a distribution signing key, but I may make Windows binaries available at some point since those are easier to do.

To use S3 (at least for the intended use case), you'll need S3 and a file (or files) with oscilloscope visuals that are rendered in monotone at 55Hz, AKA A1. Then, simply click the load button at the top, navigate to the file, and open it. The rest of the controls should™ be self-explanatory.

The automation interface is as follows:
- Slot # - Which sample slot will play when you press a MIDI note
- Reset Current - Whenever this changes, the current sample will be reset to the start
- Reset All - Whenever this changes, all loaded samples will be reset to the start
