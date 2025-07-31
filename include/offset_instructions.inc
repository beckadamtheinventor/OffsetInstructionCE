
; this must be invoked before any instructions and there must not be a format directive
; ti84pceg.inc must be included
macro flash_executable? temp_location_4b:ti.cursorImage, temp_location_3b:ti.scrapMem, temp_location_9b:ti.cursorImage+4
    local programexecbase, __handle_offset_instruction
    local _data, _header, _end, _start, _cleanup_code, _ram_code
    element programexecbase
    __handle_offset_instruction := temp_location_4b

	; dq '**TI83F*'
	; db 26, 10, 0
	; rb 42
	; dw _end - $ - programexecbase
	; dw _start - programexecbase - _header
    ; dw _end - _start
    ; _header:
	; db ti.ProtProgObj
	; dq name
	; db 0, 1 shl 7
	; dw _end - _data - programexecbase
	; _data:
	; dw _end - _var - programexecbase
	; _var:
    dw 07BEFh
	; postpone ?
	; 	checksum = 0
	; 	repeat _end - _start
	; 		load temp: byte from: _header + %-1
	; 		checksum = (checksum + temp) and 0FFFFh
	; 	end repeat
	; 	dw checksum
	; end postpone

    virtual at programexecbase
        _start:
        ld iy,temp_location_4b
        ld (iy+0),$E1 ; pop hl
        ld (iy+1),$E5 ; push hl
        ld (iy+2),$C9 ; ret
        call temp_location_4b ; get address of following instruction in hl
        ld de,_ram_code-$
        add hl,de ; HL = address of ram code
        ex hl,de
        ld iy,0
        add iy,sp
        lea hl,iy-_ram_code.len
        ld sp,hl
        ex hl,de
        ld bc,_ram_code.len
        push de
        ldir ; copy the ram code to the stack
        pop de
        ; put a jump to the ram code at the temp location, which can now be called by the program
        call temp_location_4b
        push hl
        ld hl,temp_location_4b
.idk_what_to_call_this_label:
        ld (hl),$C3
        inc hl
        ld (hl),de
        pop hl
        ld de,_cleanup_code - .idk_what_to_call_this_label
        add hl,de
        push hl ; return to cleanup routine
        ; jump past the storage for the ram code and cleanup code
        jr _ram_code.end
    _cleanup_code:
        ld hl,_ram_code.len
        add hl,sp
        ld sp,hl
        ret

    _ram_code:
        push hl,de,iy,af,ix ; save hl,de,iy,af,ix
        ; sp is now 15 bytes down
        ld iy,0
        add iy,sp
        ld hl,(iy+15) ; grab pointer to caller
        ld a,(hl) ; grab opcode byte of instruction to offset
        ld ix,temp_location_9b
        ld (ix+0),0 ; nop (replaced with two-byte opcode first byte if applicable, otherwise remains nop)
        ; deliberately don't handle 0xED or 0xCB instructions because neither of them have word arguments anyways
        cp a,$DD
        jr z,.two_byte_instruction
        cp a,$FD
        jr nz,.not_two_byte_instruction
    .two_byte_instruction:
        ld (ix+0),a
        inc hl
        ld a,(hl)
    .not_two_byte_instruction:
        inc hl
        ld de,(hl) ; opcode argument word
        inc hl
        inc hl
        inc hl
        ex hl,de
        add hl,de ; offset the argument by its address
        ld (ix+1),a ; opcode byte
        ld (ix+2),hl ; offset opcode argument word
        ld (temp_location_3b),ix

        cp a,$CD
        jr z,.opcode_is_call
        sub a,$C4 ; lowest call opcode byte
        jr c,.opcode_is_not_call
        and a,$F
        jr z,.opcode_is_call
        cp a,8
        jr z,.opcode_is_call
    .opcode_is_not_call:
        ld (ix+5),$C3 ; jp
        ld (ix+6),de
        ld (iy+15),ix
        pop ix,af,iy,de,hl
        ret
    .opcode_is_call:
        ld (ix+5),$C9 ; ret
        ld (iy+15),de ; advance caller
        pop ix,af,iy,de
        ld hl,(temp_location_3b)
        ex (sp),hl ; restore hl, push jump address
        ret
    _ram_code.len:=$-_ram_code
    _ram_code.end:

    macro end?.flash_executable?
        _end:
        local data
        load data: $-$$ from $$
        end virtual
        db data
        restore flash_executable
    end macro
    macro ? line&
        match opcode= args, line
            if `opcode = "call" | `opcode = "jp" | `opcode = "ld"
                match lhs=,rhs, args
                    match (val), lhs
                        if val relativeto programexecbase
                            call __handle_offset_instruction
                            opcode (0), rhs
                            store val - $ : 3 at $ - 3
                        else
                            opcode (val), rhs
                        end if
                    else match (val), rhs
                        if val relativeto programexecbase
                            call __handle_offset_instruction
                            opcode lhs, (0)
                            store val - $ : 3 at $ - 3
                        else
                            opcode lhs, (val)
                        end if
                    else if rhs relativeto programexecbase
                        call __handle_offset_instruction
                        opcode lhs, 0
                        store rhs - $ : 3 at $ - 3
                    else
                        opcode lhs, rhs
                    end if
                else match opcode= lhs, line
                    match (val), lhs
                        opcode lhs
                    else if lhs relativeto programexecbase
                        call __handle_offset_instruction
                        opcode 0
                        store lhs - $ : 3 at $ - 3
                    else
                        opcode lhs
                    end if
                else
                    opcode lhs,rhs
                end match
            else
                opcode args
            end if
        else
            line
        end match
    end macro
end macro
