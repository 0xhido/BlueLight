# BlueLight 

BlueLight is an open-source kernel component for kernel-mode process activity monitoring and setup for user-mode API calls monitoring.

## Goals

The goal of this project is to create a Windows kernel component for EDR system, specifically, [BLUESPAWN](https://github.com/ION28/BLUESPAWN) - an open-source EDR.

## Architecture 

BlueLight built using file-system mini-filter driver which sends events to user-mode over communication port.

In addition, the driver uses `injdrv` for injecting custom DLL to every thread (right after loading `kernel32.dll`).

## Monitoring

Currently implemented:

- Process Creation / Termination 
- Thread Creation / Termination
- Remote Thread Creation
- Image Loading

## Acknowledgements

- [Injdrv](https://github.com/wbenny/injdrv) 
- [BLUESPAWN](https://github.com/ION28/BLUESPAWN)
- [hidden](https://github.com/JKornev/hidden)