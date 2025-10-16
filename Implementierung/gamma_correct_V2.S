.intel_syntax noprefix
.global gamma_correct_V2
.text

gamma_correct_V2:
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
  pxor xmm10, xmm10


  //255 in float hex
  mov r11, 0x437F0000
  movd xmm10, r11

  //counter 
  xor r12, r12

  //xmm4 = a + b + c
  movss xmm4, xmm5
  addss xmm4, xmm6
  addss xmm4, xmm7
  //a+b+c * 255, so that we can divide only once, not twice
  mulss xmm4, xmm10

  //elements count
  imul rsi, rdx 

  mov r13, rsi
  mov r14, rdi
  mov r15, rcx

  sub rsp, 0x60


  .Lloop:
  cmp r12, r13
  jae .Lend

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
  movdqu [rsp + 0x50], xmm10

  call power_V1

  movdqu xmm1, [rsp]
  movdqu xmm4, [rsp + 0x10]
  movdqu xmm5, [rsp + 0x20]
  movdqu xmm6, [rsp + 0x30]
  movdqu xmm7, [rsp + 0x40] 
  movdqu xmm10, [rsp + 0x50] 


  // * 255
  xor rdx, rdx
  mulss xmm0, xmm10
  cvtss2si rdx, xmm0

  mov byte ptr [r15 + r12], dl

  add r12, 1
  add r14, 3
  jmp .Lloop

  //.Lnull:
  //jmp .Lloop

  .Lend:

  add rsp, 0x60

  pop r15
  pop r14
  pop r13
  pop r12
  pop rbx
  ret
