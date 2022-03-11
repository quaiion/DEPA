push 2048 / videoseg start
call setver
stdmp
call sethor
stdmp
/
push 487 / 162 * 3 + 1
sub
pop dx
/
stdmp
call horlin
push dx
push 156 / 162 - 6
add
pop dx
stdmp
call corn
push dx
push 160 / 162 - 2
add
pop dx
stdmp
call verlin
push dx
push 2
add
pop dx
stdmp
call corn
push dx
push 165 / 162 + 3
add
pop dx
stdmp
call horlin
/
prnt
/
hlt
/
/
setver:
in / vert coord
push 1
stdmp
sub
push 162
stdmp
mul
add
stdmp
ret
/
sethor:
in / hor coord
push 1
sub
add
ret
/
horlin:
push 3
push 0
hls:
push 1
pop [dx]
push dx
push 1
add
pop dx
push 1
add
ja hls
pop
pop
ret
/
corn:
push 1
push 1
pop [dx]
pop [dx+8]
ret
/
verlin:
push 3
push 0
vls:
push 1
push 1
pop [dx]
pop [dx+12]
push dx
push 162
add
pop dx
push 1
add
ja vls
pop
pop
ret
