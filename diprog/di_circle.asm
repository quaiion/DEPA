push 2048.000
call 000000B5
stdmp
call 000000DF
stdmp
push 487.000
sub
pop dx
stdmp
call 000000E8
push dx
push 156.000
add
pop dx
stdmp
call 00000115
push dx
push 160.000
add
pop dx
stdmp
call 00000128
push dx
push 2.000
add
pop dx
stdmp
call 00000115
push dx
push 165.000
add
pop dx
stdmp
call 000000E8
prnt
hlt
in
push 1.000
stdmp
sub
push 162.000
stdmp
mul
add
stdmp
ret
in
push 1.000
sub
add
ret
push 3.000
push
push 1.000
pop [dx]
push dx
push 1.000
add
pop dx
push 1.000
add
ja 000000F2
pop
pop
ret
push 1.000
push 1.000
pop [dx]
pop [dx+8]
ret
push 3.000
push
push 1.000
push 1.000
pop [dx]
pop [dx+12]
push dx
push 162.000
add
pop dx
push 1.000
add
ja 00000132
pop
pop
ret
