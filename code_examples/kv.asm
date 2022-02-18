push 0
in
in
in
pop cx
pop bx
pop ax
push bx
push bx
mul
stdmp
push ax
push cx
push 4
mul
mul
sub
je single
ja none
call sqrt
pop dx
push bx
sub
pop bx
push bx
push dx
add
push 2
push ax
push bx
push dx
sub
push 2
push ax
mul
div
out
mul
div
out
none:
hlt
/
/
single:
push bx
sub
push 2
push ax
mul
div
out
hlt
/
/
sqrt:
pop cx
push cx
push 2
div
pop [0]
push 0
push 16
iter:
pop
pop dx
push 0.5
push [0]
push cx
push [0]
div
add
mul
pop [0]
push dx+1
push 16
jbe iter
pop
pop
push [0]
ret
