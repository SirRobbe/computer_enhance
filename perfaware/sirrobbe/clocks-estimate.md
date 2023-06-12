bits 16

mov bx, 1000 // Register immediate 4 sum: 4
mov bp, 2000 // Register immediate 4 sum: 8
mov si, 3000 // Register immediate 4 sum: 12
mov di, 4000 // Register immediate 4 sum: 16

mov cx, bx   // Register Register 2 sum: 18
mov dx, 12   // Register immediate 4 sum: 22

mov dx, [1000] // 8 + EA (Displacement only 6) 8 + 6 sum: 36

mov cx, [bx] // 8 + EA (bx only 5) 8 + 5 sum: 49
mov cx, [bp] // 8 + EA (bx only 5) 8 + 5 sum: 62
mov [si], cx // 9 + EA (si only 5) 9 + 5 sum: 76
mov [di], cx // 9 + EA (di only 5) 9 + 5 sum: 90

mov cx, [bx + 1000] // 8 + EA (bx + displacement 9) 8 + 9 sum: 107
mov cx, [bp + 1000] // 8 + EA (bp + displacement 9) 8 + 9 sum: 124
mov [si + 1000], cx // 9 + EA (si + displacement 9) 9 + 9 sum: 142
mov [di + 1000], cx // 9 + EA (di + displacement 9) 9 + 9 sum: 160

add cx, dx // 3 sum: 163
add [di + 1000], cx // 16 + EA (di + displacement 9) 16+9 sum: 188
add dx, 50 // 4 sum: 192
