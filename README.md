# Fractal Sound Explorer
### Continuous Wave Mod by rwvens

This slight modification of the original code changes the audio generation code to generate a sine wave with frequency set to the imaginary component of the iteration variable and magnitude set to the distance from the mean. This enables coherent and interesting audio in a wider set of of situations.

The idea is based on the fact that multiplication of complex numbers causes the vectors to rotate, creating a natural analogy for generating audio. Since the rotation is based on the value of the imaginary component, it makes sense to use *i* to control the pitch of the output wave.

Demo Video:

https://youtu.be/fRS4zQu95O0

### Original

Explore fractals in an audio-visual sandbox

Download executable on my itch.io page:
https://codeparade.itch.io/fractal-sound-explorer

YouTube video explanation:
https://youtu.be/GiAj9WW1OfQ

![](https://img.itch.zone/aW1hZ2UvOTM1NzMzLzUzMTU0MzEucG5n/original/ay7ju0.png)

Notes
---------------
The fractals are designed to run in real time on a GPU.  If the program is going too slow, you can simply shrink the size of the window to make it run faster.  The rendering is also designed to increase the image quality over time.  So keep still for a bit before taking screenshots to get higher quality.

Controls
---------------
* H - Toggle Help Menu
* Left Mouse - Click or drag to hear orbits
* Middle Mouse - Drag to pan view
* Right Mouse - Stop orbit and sound
* Scroll Wheel - Zoom in and out
* D - Toggle Audio Dampening
* C - Toggle Color
* F11 - Toggle Fullscreen
* S - Save Snapshot
* R - Reset View
* J - Hold down, move mouse, and release to make Julia sets. Press again to switch back.
* 1 - Mandelbrot Set
* 2 - Burning Ship
* 3 - Feather Fractal
* 4 - SFX Fractal
* 5 - Hénon Map
* 6 - Duffing Map
* 7 - Ikeda Map
* 8 - Chirikov Map
