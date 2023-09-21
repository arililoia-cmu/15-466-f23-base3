# Angle Matcher 2: Negative Reinforcement

Author: Ari Liloia

Design: This game is a continuation of the "angle matcher" game I submitted for game 2, but in this version there is background music, as well as a a synthesized voice saying "no" faster and faster as the correct angle is approached and "correct" when the user earns a point. The new sounds make the game more stressful (a good thing).

My original plan for this game was to give the user the ability to draw a waveform on the screen - it took me a long time to understand how I could edit buffer values individually - see the generate_audio function in PlayMode.cpp, and the new call to drawlines that draws just one line on the screen (also took a long time to figure out how to draw in window space) - if I had more time I would have implemented this. I also wanted to implement picking as I wasn't able to figure out how to do this in time for game 2.

I couldn't do this in time, so I put together a new version of my game 2 that incorporates sound (but not in the way I intended) and picking. I created the background music, and I used a license-free text to speech tool built into the software WavePad Sound Editor to synthesize the voices. 

Screen Shot:

![Screen Shot](screenshot.png)

How To Play:
Click the knob and move around it (use WASD to move in space, escape to lock the camera and be able to click around the screen) to the match the angle at the bottom of the screen. When the angle is matched, you get a point. Play as long as you want

This game was built with [NEST](NEST.md).
