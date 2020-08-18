; file ps.as
.entry LIST
.entry LIST_INVALID##
.extern W
MAIN: add r3, LIST#
LOOP: prn #48
LOOP!!: prn #48
prn #99999999999999999999
 lea W,    r6
 inc K
 mov r3, K, EXTRA
 sub r1,  VVVVV
 bne END
 cmp K, #-6
 bne &END
 dec W
 dec HAHAHAHA
.entry MAIN
 jmp &LOOP
 add L3, L3
END: stop
STR: .string "abcd"
LIST: .data 6,,,
LIST: .data 6,, -99999999999999999
LIST2: .data## X, -9
.data 6, -99999999999999999
LIST_INVALID##: .data 10
    .data -100
K: .data 31
.extern L3
