Welcome to
 ________  ___       ________  ________  ________      
|\   __  \|\  \     |\   __  \|\   __  \|\   ____\     
\ \  \|\  \ \  \    \ \  \|\  \ \  \|\  \ \  \___|_    
 \ \   __  \ \  \    \ \   ____\ \   __  \ \_____  \   
  \ \  \ \  \ \  \____\ \  \___|\ \  \ \  \|____|\  \  
   \ \__\ \__\ \_______\ \__\    \ \__\ \__\____\_\  \ 
    \|__|\|__|\|_______|\|__|     \|__|\|__|\_________\
                                           \|_________|
                                                       
Build Instructions
________________________

For Mac, navigate to the mac folder and run the following command:
$ rm alpas-main & rm alpas-keybinder & rm alpas-engine & make alpas-main && ./alpas-main

On Windows, navigate to the windows folder and compile alpas-main.c on any designated compiler

Usage
________________________

For mac, run the compiled binary by running
$ ./alpas-main

For windows, launch alpas-main.exe

Structure of Directory 
________________________

windows - Windows Files
	|- alpas-engine.c (Rendering math helper functions)
	|- alpas-keybinder.c (For binding keystrokes to game)
	|- alpas-main.c (Game logic, everything)
	|- alpas-main.exe (Main binary, in the case some libraries aren't supported on user end)


mac - Mac Files
	|- alpas-engine.c (Rendering math helper functions)
	|- alpas-keybinder.c (For binding keystrokes to game)
	|- alpas-main.c (Game logic, everything)
	|- alpas-main (Mac binary)
