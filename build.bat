@ECHO OFF

SET WCL386=-zq -zdp -wcd=138 -ecc -4s -mf -fp3 -od -d2

NASM src\main.asm -fobj -o obj\mainasm.obj -g
NASM src\kbd.asm -fobj -o obj\kbdasm.obj -g
NASM src\modex.asm -fobj -o obj\modexasm.obj -g
NASM src\radplay.asm -fobj -o obj\radasm.obj -g

REM Pause here to allow you to process any error messages from the assembler
PAUSE

REM Now we compile our C source
WCL386 main.c -c -fo=obj\main.obj
WCL386 src\ipackldr.c -c -fo=obj\ipackldr.obj
WCL386 src\sound.c -c -fo=obj\sound.obj
WCL386 src\timer.c -c -fo=obj\timer.obj
WCL386 smix\LOWMEM.c -c -fo=obj\LOWMEM.obj
WCL386 smix\SMIX.c -c -fo=obj\SMIX.obj

REM Check for error files and stop if we find any
IF EXIST *.ERR GOTO CompilerError
GOTO CompilerOK

:CompilerError
    ECHO COMPILATION FAILED: ERRORS FOUND!
    ECHO =-------------------------------=
    ECHO List of error files:
    DIR
    GOTO EndOfBatch

:CompilerOK
    REM Link the EXE
    ECHO Compilation OK
    ECHO =------------=
    WCL386 obj\*.obj -fe=GAME.EXE

ECHO Done!

:EndOfBatch