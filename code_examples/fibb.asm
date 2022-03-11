push 1
push 1
push 2
in
push 1
je one
pop
call fibb
one:
pop
pop
out
hlt
/
/
fibb:
je end
pop ax
pop bx
pop cx
push cx
add
pop dx
push cx
push dx
push bx+1
push ax
call fibb
end:
ret
