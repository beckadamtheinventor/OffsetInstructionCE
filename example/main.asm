include "../include/ez80.inc"
include "../include/ti84pceg.inc"
include "../include/offset_instructions.inc"

flash_executable

    call ti.RunIndicOff
    call ti.HomeUp

    ld hl,str_HelloWorld
    call _print

    call ti.RunIndicOn
    jp ti.DrawStatusBar

_print:
    push hl
    call ti.os.PutStrLine
    pop hl
    jp ti.NewLine

str_HelloWorld:
    db "Hello World!",0

end flash_executable

