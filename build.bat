@ECHO OFF

SET WCL386=-zq -zdp -wcd=138 -ecc -4s -mf -fp3 -od -d2

NASM src\main.asm -fobj -o obj\mainasm.obj -g

REM Pause here to allow you to process any error messages from the assembler
PAUSE

REM Now we compile our C source
WCL386 src\main.c -c -fo=obj\main.obj
WCL386 src\ipackldr.c -c -fo=obj\ipackldr.obj

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