.intel_syntax noprefix
.global gamma_correct_V1
.text

gamma_correct_V1:
  //rdi - pointer to img
  //rsi - width
  //rdx - height
  //xmm0,1,2,3 - a,b,c,gamma
  //rcx - pointer to result

  push rbx
  push r12
  push r13
  push r14
  push r15
  
  //a - xmm5. b - xmm6, c - xmm7, xmm1 - gamma
  movss xmm5, xmm0
  movss xmm6, xmm1
  movss xmm7, xmm2
  movss xmm1, xmm3

  xor r11, r11
  pxor xmm4, xmm4
  pxor xmm2, xmm2

  //xmm4 = a + b + c
  movss xmm4, xmm5
  addss xmm4, xmm6
  addss xmm4, xmm7

  //elements count
  imul rsi, rdx

  mov r13, rsi
  mov r14, rdi
  mov r15, rcx

  sub rsp, 0x60

  //255 in float hex
  mov r11, 0x437F0000
  movd xmm2, r11

  mulss xmm4, xmm2

  test r13, 3
  jz .Lloopstart

  //counter
  xor r12, r12

  pxor xmm0, xmm0
  xor rdx, rdx

  .Lunaligned:
  cmp r13, 0
  jle .Lend

  xor rdx, rdx
  pxor xmm8, xmm8
  pxor xmm9, xmm9

  //red
  mov dl, byte ptr [r14]
  cvtsi2ss xmm0, rdx
  mulss xmm0, xmm5

  //green
  mov dl, byte ptr [r14 + 1]
  cvtsi2ss xmm8, rdx
  mulss xmm8, xmm6

  //blue
  mov dl, byte ptr [r14 + 2]
  cvtsi2ss xmm9, rdx
  mulss xmm9, xmm7

  //a*R + b*G + c*B
  addss xmm0, xmm8
  addss xmm0, xmm9

  // D / 255
  divss xmm0, xmm4

  movdqu [rsp], xmm1
  movdqu [rsp + 0x10], xmm4
  movdqu [rsp + 0x20], xmm5
  movdqu [rsp + 0x30], xmm6
  movdqu [rsp + 0x40], xmm7
  movdqu [rsp + 0x50], xmm2

  call power_V1

  movdqu xmm1, [rsp]
  movdqu xmm4, [rsp + 0x10]
  movdqu xmm5, [rsp + 0x20]
  movdqu xmm6, [rsp + 0x30]
  movdqu xmm7, [rsp + 0x40]
  movdqu xmm2, [rsp + 0x50] 

  xor rdx, rdx
  mulss xmm0, xmm2
  cvtss2si rdx, xmm0
 
  mov byte ptr [r15 + r12], dl


  sub r13, 1
  add r12, 1
  add r14, 3

  test r13, 3
  jz .Lloopstart
  jmp .Lunaligned


  .Lloopstart:
  xor r12, r12
  //make a register with 4 values

  //xmm:1 - gamma, 2 - 255, 4 - a+b+c * 255, 5 - a, 6 - b, 7 - c
  pshufd xmm1, xmm1, 0
  pshufd xmm2, xmm2, 0
  pshufd xmm4, xmm4, 0
  pshufd xmm5, xmm5, 0
  pshufd xmm6, xmm6, 0 
  pshufd xmm7, xmm7, 0

  .Lloop:
  cmp r12, r13
  jae .Lend

  //xmm5 (starts with red), xmm6 (g), xmm7 (b) - next 4 pixels

  pmovzxbd xmm0, dword ptr [r14]
  pmovzxbd xmm8, dword ptr [r14 + 4]
  pmovzxbd xmm9, dword ptr [r14 + 8]

  //xmm10 - copy of xmm8
  movdqu xmm10, xmm8

   //xmm11 - copy of xmm9
  movdqu xmm11, xmm9

  //xmm0 - r0 g0 b0 r1, red
  insertps xmm9, xmm0, 0x40
  insertps xmm8, xmm0, 0x80
  insertps xmm0, xmm0, 0xd0
  //xmm0 - r0 r1 x  x
  //xmm8 - b0 b1 x  x
  //xmm9 - g0 x  g3 x

  //xmm8 - g1 b1 r2 g2, blue
  insertps xmm0, xmm10, 0xa0
  insertps xmm9, xmm10, 0xe0

  //xmm0 - r0 r1 r2 x
  //xmm8 - b0 b1 x  x
  //xmm9 - g0 x  g2 x 

  //xmm9 - b2 r3 g3 b3, green
  insertps xmm8, xmm11, 0x20
  insertps xmm8, xmm11, 0xf0
  insertps xmm0, xmm11, 0x70
  //xmm0 - r0 r1 r2 r3
  //xmm8 - b0 b1 b2 b3
  //xmm9 - g0 x  g2 x 

  insertps xmm9, xmm10, 0x10
  insertps xmm9, xmm11, 0xb0

  //xmm0 - r0 r1 r2 r3
  //xmm8 - b0 b1 b2 b3
  //xmm9 - g0 g1 g2 g3 


  cvtdq2ps xmm0, xmm0
  cvtdq2ps xmm8, xmm8
  cvtdq2ps xmm9, xmm9


  //a * R
  mulps xmm0, xmm5
  //b * G
  mulps xmm9, xmm6
  //c * B
  mulps xmm8, xmm7


  //a*R + b*G + c*B
  addps xmm0, xmm9
  addps xmm0, xmm8

  // D / 255 
  divps xmm0, xmm4

  movdqu [rsp], xmm1
  movdqu [rsp + 0x10], xmm4
  movdqu [rsp + 0x20], xmm5
  movdqu [rsp + 0x30], xmm6
  movdqu [rsp + 0x40], xmm7
  movdqu [rsp + 0x50], xmm2

  call powerSIMD_V1

  movdqu xmm1, [rsp]
  movdqu xmm4, [rsp + 0x10]
  movdqu xmm5, [rsp + 0x20]
  movdqu xmm6, [rsp + 0x30]
  movdqu xmm7, [rsp + 0x40] 
  movdqu xmm2, [rsp + 0x50]

  mulps xmm0, xmm2

  // convert floats to dword integers
  cvtps2dq xmm0, xmm0

  //convert dword integers to byte integers
  pshufb xmm0, xmmword ptr [rip + .Lmask]
  movd dword ptr [r15 + r12], xmm0

  add r12, 4
  add r14, 12
  jmp .Lloop

  .Lend:
  add rsp, 0x60
  pop r15
  pop r14
  pop r13
  pop r12
  pop rbx
  ret

.section .rodata
  .align 16      
  .Lmask:
    .byte 0,4,8,12
    .rept 12
    .byte 0xFF
    .endr