# Untitled Megadrive Game
> This is a build pipeline for Sega Megadrive/Genesis games. The output of the demo is 'sonic' from the SGDK samples folder. We build a docker iamge to compile the ROM, docker is a requirement, not a suggestion.

Any example code used here (Sonic sprites and test music for example) is for learning purposes, no copyright infringement intended.

## You need
- Docker
- Windows

## I recommend
- Visual Studio Code

## Quick Build in VSCode
`ctrl` + `shift` + `b`

## Powershell Scripts in `./scripts`
- Prepare, Clean, Build, and Run - `build-run.ps1`
- Prepare Docker image - `prepare.ps1`
- Clean build folder - `clean.ps1`
- Build ROM - `build.ps1`
- Run ROM in emulator - `run.ps1`

## Thank you to
- https://github.com/Stephane-D/SGDK
- http://www.gens.me/ and https://segaretro.org/Gens_KMod