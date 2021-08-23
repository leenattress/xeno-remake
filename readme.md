# 🎮 Megadrive Game Starter

> ℹ - This is a build pipeline for Sega Megadrive/Genesis games. We build a docker image to compile the ROM, docker is a requirement, not a suggestion.

## 🎉 You need
- Docker
- Windows

## 🤔 Recommended
- Visual Studio Code

The output of this starter is `sonic` from the SGDK samples folder. Any example code used here (Sonic sprites and test music for example) is for learning purposes, no copyright infringement intended. You will find the games source in `/src/main.c` and the sprites and resources in `/res`. 

The build scripts try their best to download and prepare things you need before it uses them, so you should be able to pull down this repo in VSCode and press `ctrl` + `shift` + `b` to build and run the game.

## SGDK Documentation
- https://github.com/Stephane-D/SGDK/wiki/Tuto-Introduction
- https://www.ohsat.com/tutorial/#mega-drive-tutorials

## SGDK Basic Game Structure

```c
#include <genesis.h>

int main(u16 hard)
{
    // run these things once at startup

    while(TRUE) // loop forever
    {
        // read input
        // move sprite
        // update score
        // draw current screen (logo, start screen, settings, game, gameover, credits...)
    }

    return 0;
}
```

## Included

- Extremely simple way to get started with Megadrive/Genesis development
- Sample `sonic` from `SGDK` framework.
- Scripts to build and run the ROM
- VSCode `tasks.json` to enable `ctrl` + `shift` + `b`
- GitHub Actions to build ROM in pipeline
- Gens KMod emulator, with advanced debugging features added

## Powershell Scripts in `/scripts`
- Prepare, Clean, Build, and Run - `build-run.ps1`
- Prepare Docker image - `prepare.ps1`
- Clean build folder - `clean.ps1`
- Build ROM - `build.ps1`
- Run ROM in emulator - `run.ps1`

## Thank you to
- https://github.com/Stephane-D/SGDK
    - Please [support SGDK on Patreon](https://www.patreon.com/SGDK)
- http://www.gens.me/ and https://segaretro.org/Gens_KMod

## TL;DR - Build and Run in VSCode
`ctrl` + `shift` + `b`