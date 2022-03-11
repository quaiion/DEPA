push 1.000
push 1.000
push 2.000
in
push 1.000
je 00000028
pop
call 0000002C
pop
pop
out
hlt
je 00000055
pop ax
pop bx
pop cx
push cx
add
pop dx
push cx
push dx
push bx+1.000
push ax
call 0000002C
ret
