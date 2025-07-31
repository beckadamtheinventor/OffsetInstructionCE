# Offset Instructions CE

Allow assembly programs to run directly in flash!
(if those programs don't use self-modifying code)


## Usage

```
include "ez80.inc"
include "ti84pceg.inc"
include "tiformat.inc"
include "offset_instructions.inc"

flash_executable
    ; your code here
end flash_executable

```



## How it works

Magic!
Long story short, instructions with a 3-byte word argument are assembled differently.

```
call some_function_in_my_program
```

would translate to

```
call __handle_offset_instruction
call some_function_in_my_program - $
```

The call to `__handle_offset_instruction` is automatically inserted if the address you're referencing lies within the program.
This function uses the return address pushed to the stack (when it gets called) and adds that value to the opcode argument of the following instruction.


Currently it handles:
- call and conditional call, as well as using an offset call while within an offset call
- jp and conditional jp
- ld variants with a 3-byte word argument

