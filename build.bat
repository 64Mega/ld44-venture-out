@ECHO OFF

SET WCL386=-zq -zdp -wcd=138 -ecc -4s -mf -fp3 -od -d2

REM Do Asset packaging
TOOLS\IPACK break.lst break.pak

NASM src\main.asm -fobj -o obj\mainasm.obj -g
NASM src\kbd.asm -fobj -o obj\kbdasm.obj -g
NASM src\modex.asm -fobj -o obj\modexasm.obj -g
NASM src\radplay.asm -fobj -o obj\radasm.obj -g

REM Pause for any assembly errors
PAUSe

REM Now we compile our C source
WCL386 main.c -c -fo=obj\main.obj
WCL386 src\ipackldr.c -c -fo=obj\ipackldr.obj
WCL386 src\sound.c -c -fo=obj\sound.obj
WCL386 src\timer.c -c -fo=obj\timer.obj
WCL386 src\pal.c -c -fo=obj\pal.obj
WCL386 smix\LOWMEM.c -c -fo=obj\LOWMEM.obj
WCL386 smix\SMIX.c -c -fo=obj\SMIX.obj

REM Game-logic specific compilation
WCL386 game\data.c -c -fo=obj\data.obj
WCL386 game\text.c -c -fo=obj\text.obj
WCL386 game\s_game.c -c -fo=obj\s_game.obj
WCL386 game\entities\paddle.c -c -fo=obj\paddle.obj
WCL386 game\entities\ball.c -c -fo=obj\ball.obj
WCL386 game\entities\effect.c -c -fo=obj\effect.obj
WCL386 game\entities\bush.c -c -fo=obj\bush.obj
WCL386 game\entities\rock.c -c -fo=obj\rock.obj
WCL386 game\entities\cliff.c -c -fo=obj\cliff.obj
WCL386 game\entities\chest.c -c -fo=obj\chest.obj

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