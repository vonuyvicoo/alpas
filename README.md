# ALPAS - to break free. 3D Terminal Game for Mac/Windows
Built for CMSC11 class requirement. 3D Rendering is done using mainly linear algebra and trigonometric derivations. Read full process [here][https://raw.githubusercontent.com/vonuyvicoo/alpas/main/ALPAS%20MAIN%20DOCUMENTATION.pdf]

![License](https://img.shields.io/badge/License-MIT-yellow.svg)
![Static Badge](https://img.shields.io/badge/C_Programming_Language-blue)

## Build Instructions
________________________

For Mac, navigate to the mac folder and run the following command:
```sh
$ rm alpas-main & rm alpas-keybinder & rm alpas-engine & make alpas-main && ./alpas-main
```

On Windows, navigate to the windows folder and compile `alpas-main.c` on any designated compiler.

## Usage
________________________

For Mac, run the compiled binary by running:
```sh
$ ./alpas-main
```

For Windows, launch `alpas-main.exe`.

## Structure of Directory
________________________

**windows** - Windows Files
- `alpas-engine.c` (Rendering math helper functions)
- `alpas-keybinder.c` (For binding keystrokes to game)
- `alpas-main.c` (Game logic, everything)

**mac** - Mac Files
- `alpas-engine.c` (Rendering math helper functions)
- `alpas-keybinder.c` (For binding keystrokes to game)
- `alpas-main.c` (Game logic, everything)

## Areas for Improvement
________________________

- Optimize printing by using a buffer and printing the buffer as string
- Add compatibility to Windows

## License
________________________

This project is licensed under the MIT License. See the [LICENSE](LICENSE.md) file for details.

