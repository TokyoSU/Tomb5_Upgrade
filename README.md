This project use a new version from [TOMB5](https://github.com/Trxyebeep/TOMB5) by Troye and include new features and improvements the game needed.

# New Features

### - FMV_PLAYER
New fmv player that don't need the game to resize to play.

### - SDL2
The game now use SDL2 which manage both the window, input and audio:
Through the audio part is only for the FMV, the game now use the input of SDL2 instead of DInput8.
All part which used WIN32 for the window was removed.

### - CONFIG
Now the game don't use the registry anymore,
everything is done through the config.ini file which you can change if you need it to.

### - DDRAW_COMPAT
Now the game use [DDrawCompat](https://github.com/narzoul/DDrawCompat) to add fix and features for the old DDraw/DirectX 1 to 7.
Through this option would be removed if the project move to DirectX9 or more.
