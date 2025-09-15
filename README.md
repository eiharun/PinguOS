# PinguOS

[Wiki](https://github.com/eiharun/PinguOS/wiki)

## [Using this Playlist as a reference](https://www.youtube.com/playlist?list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M)

## How to run

Clone the repo, install all [dependencies](#dependencies), and run

```sh
> make run
```

and it should build and run qemu

![nootnootqemu](/ScreenShots/nootnoot.gif)

## Verions

- 0.1.5: Mouse driver
- 0.1.4: Keyboard driver
- 0.1.3: Added ports.h, communicating with hardware, improved printf, compile into build dir
- 0.1.2: Added global descriptor table
- 0.1.1: Fixed calling constructors for static and global objects. Added types.h
- 0.1.0: Prints `Noot Noot` to the screen

## Dependencies

- sudo apt-get install build-essential (obvious)
- sudo apt-get install grub-pc-bin grub-common xorriso (grub bootloader -> kernel image)
- sudo apt-get install qemu qemu-system-i386 qemu-utils (virtualization)
