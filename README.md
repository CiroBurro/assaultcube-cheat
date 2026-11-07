# Yet another assault cube cheat
This repo stores the source code, my cheat table and the IDA Pro db I made while writing my first videogame hack!

<img width="960" height="540" alt="ac_cheat" src="https://github.com/user-attachments/assets/acb3804c-d8a1-48c5-9ba5-9f6f03b06c23" />

## DISCLAIMER!
This project is purely for educational purposes only: assault cube is a single-player game and I, as a gamer myself, do not intend to ruin other's people game experience.

## Features
As you can see in the picture above, this cheat implements two different hacks:
1. **God mode**
2. **ESP**

Both of them can be toggled via an overlay menu (press F1 to open it)

### God Mode
This hack let the player become invincible by enabling infinite health, armor and ammo for both the rifle and the pistol.
I did not include other weapons compatibility, such as the sniper, since this is only a demo to show how a God Mode works

### ESP (Extra Sensory Perception)
This hack let the player discover many information he should not know: enemies' health, equipment and especially position (**wallhack**).
To work properly this hack need the game run in **windowed fullscreen** otherwise the overlay will not appear or enemy position won't be correct

## Technical details
The idea is very simple: find the address in memory of the data you want to read/write, find an offset or a static pointer to that address so that you can reach it every time you launche the game bypassing protections like ASLR, automate the process of reading/writing the game's memory using a programming language.

In my case I used the Win32 API, especially: `ReadProcessMemory` and `WriteProcessMemory`, wrapped in a custom class `Memory`, which exposes several ways to read/write custom data through template methods with an intuitive syntax:
```c++
int x = mem.Read<int>(address);
mem.Write<int, 5>(address, std::array<int, 5>{1, 2, 3, 4, 5});
mem.Patch(address, "\x01\x73\x04");
```

I also wrote a `Process` class: a wrapper for a `HANDLE` data structure to correctly implement the RAII paradigm, with also some utility methods like:
```c++
const uintptr_t getModuleBaseAddress(const std::string_view moduleName)
```
to get the module base address of the specified module ("ac_client.exe" in this case)

## Motivation
Well I love videogames, reverse engineering and messing up with memory so this project is perfect for me.
I wanted to improve my RE skill but still code something at the same time, for sure there are lots of ctf and crackmes to do, but I was looking for something different.
So this is it: games + hacking = cheats

## Disclaimer pt.2: AI usage
AI was used in this project to perform exactly these specific tasks:
- Entirely write imgui_backend.h/cpp: I don't know anything about windows rendering and graphics backends/api, and currently I'm not even interested in these topics, so I let AI do that for me
- `world_to_screen` function: Simply I did not know how this conversion was performed at a math level and how to implement it in c++. I found some versions online of the same function that used to matrices called "view matrix" and "projection matrix", but I don't have neither of them, so I asked AI to help me perform the translation with the data I had. Afterwards I asked AI several times to explain to me how this process works till I finally understand it (test me if you want)
- Solve some issues with cmake and build errors

The entire reverse engineering and memory scanning process was done by me.
