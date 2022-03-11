push
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
push 4.000
mul
mul
sub
je 00000069
ja 00000068
call 00000077
pop dx
push bx
sub
pop bx
push bx
push dx
add
push 2.000
push ax
push bx
push dx
sub
push 2.000
push ax
mul
divi
out
mul
divi
out
hlt
push bx
sub
push 2.000
push ax
mul
divi
out
hlt
pop cx
push cx
push 2.000
divi
pop []
push
push 16.000
pop
pop dx
push 0.500
push [
push cx
push [
divi
add
mul
pop []
push dx+1.000
push 16.000
jbe 00000090
pop
pop
push [
ret
