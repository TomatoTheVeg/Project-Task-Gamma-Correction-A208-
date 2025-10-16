.intel_syntax noprefix
.global powerSIMD
.text

//float power(float a, float b)
// c = a^b
// c = e^(b*ln(a))
// c = e^(b*ln(m*2^e))
// c = e^(b*(ln(m) + e*ln(2)))

// xmm0 - a
// xmm1 - b
// xmm15 - copy of a

powerSIMD:
    movaps xmm15, xmm0

// xmm6, r8 - bytemask to extract exponent
// xmm7, r9 - exponent offset 127

.LexponentExtraction:
    mov r8, 0x7F800000
    mov r9, 127
    movd xmm6, r8
    movd xmm7, r9
    shufps xmm6, xmm6, 0
    shufps xmm7, xmm7, 0


    pand xmm0, xmm6
    psrld xmm0, 23
    psubd xmm0, xmm7
    cvtdq2ps xmm0, xmm0
    movaps xmm14, xmm0

    // xmm14 - exponent

    // xmm6, r8 bytemask to extract mantice
    // xmm7, r9 1 (float) or 127 shifted 23 bytes left 

.Lmantice:
    
    mov r8, 0x007FFFFF
    mov r9, 0x3F800000
    movd xmm6, r8
    movd xmm7, r9
    shufps xmm6, xmm6, 0
    shufps xmm7, xmm7, 0

    pand xmm15, xmm6
    paddd xmm15, xmm7
    subps xmm15, xmm7
     // xmm15 - mantice minus 1


// xmm6-9, r8-11 minimax polynom coeffitients in float

.Lln:
    //ln(1+x) = a1*x + a2*x^2 + a3*x^3 + a4*x^4 + a5*x^5 + a6*x^6 + a7*x^7

    mov r8, 0x3F7FFEC1
    mov r9, 0xBEFFBAED
    mov r10, 0x3EA810E9
    mov r11, 0xBE675C07

    movd xmm6, r8   //a1
    movd xmm7, r9   //a2
    movd xmm8, r10  //a3
    movd xmm9, r11  //a4

    shufps xmm6, xmm6, 0
    shufps xmm7, xmm7, 0
    shufps xmm8, xmm8, 0
    shufps xmm9, xmm9, 0


    movaps xmm13, xmm15    //xmm10 - power of x (x, x^2, x^3 ...)
    pxor xmm0, xmm0   //xmm0 - answer

    
    mulps xmm6, xmm13
    addps xmm0, xmm6

    mulps xmm13, xmm15
    mulps xmm7, xmm13
    addps xmm0, xmm7

    mulps xmm13, xmm15
    mulps xmm8, xmm13
    addps xmm0, xmm8

    mulps xmm13, xmm15
    mulps xmm9, xmm13
    addps xmm0, xmm9

    // xmm0 - ln(x + 1)


  // xmm6, r8 - ln(2)
  // xmm0 - b*(ln(mantice) + exponent*ln(2))

.Lpower:
    mov r8, 0x3F317218  //ln(2)
    movd xmm6, r8
    shufps xmm6, xmm6, 0

    mulps xmm6, xmm14
    addps xmm0, xmm6
    mulps xmm0, xmm1 

    // e^k = e^(2^n * e^r)
    // n = k/ln(2) truncated
    // r = k - n*ln(2)

    // xmm6, r8 - ln(2)
    // xmm7, r9 - 1/(ln(2))
    // xmm14, r10 - 1 (float)
.Ldivision:
    mov r8, 0x3F317218  //ln(2)
    mov r9, 0x3FB8AA3B  //1/ln(2)
    mov r10, 0x3F800000 //1
    movd xmm6, r8
    movd xmm7, r9
    movd xmm14, r10
    shufps xmm6, xmm6, 0
    shufps xmm7, xmm7, 0
    shufps xmm14, xmm14, 0


    movaps xmm15, xmm0      //xmm15 - k
    mulps xmm0, xmm7
    cvttps2dq xmm0, xmm0    // xmm0 - n (int)
    movaps xmm7, xmm0       // xmm7 - n (int)
    pslld xmm7, 23
    paddd xmm14, xmm7       //xmm14 - 2^n
    
    //jmp .Lret

    cvtdq2ps xmm0, xmm0     // xmm0 - n (float)
    mulps xmm6, xmm0
    subps xmm15, xmm6       //xmm15 - r


.Lexponent:
    // xmm6-9, r8-11 taylor polynom constants
    // e^r = 1 + r + a1*r^2 + a2*r^3 + a3*r^4 + a4*r^5 + a5*r^6 + a6*r^7 + a7*r^8  +a8*r^9

    mov r8, 0x3F800000
    mov r9, 0x3F000000
    mov r10, 0x3E2AAAAB
    mov r11, 0x3D2AAAAB

    movd xmm6, r8   //1
    movd xmm7, r9   //a1
    movd xmm8, r10  //a2
    movd xmm9, r11  //a3

    shufps xmm6, xmm6, 0
    shufps xmm7, xmm7, 0
    shufps xmm8, xmm8, 0
    shufps xmm9, xmm9, 0

    movaps xmm13, xmm15
    pxor xmm0, xmm0 //xmm0 - 0

    addps xmm0, xmm6

    addps xmm0, xmm13

    mulps xmm13, xmm15
    mulps xmm7, xmm13
    addps xmm0, xmm7

    mulps xmm13, xmm15
    mulps xmm8, xmm13
    addps xmm0, xmm8

    mulps xmm13, xmm15
    mulps xmm9, xmm13
    addps xmm0, xmm9

    mov r8, 0x3C088889
    mov r9, 0x3AB60B61
    mov r10, 0x39500D01
    movd xmm6, r8   //a4
    movd xmm7, r9   //a5
    movd xmm8, r10  //a6
    shufps xmm6, xmm6, 0
    shufps xmm7, xmm7, 0
    shufps xmm8, xmm8, 0

    mulps xmm13, xmm15
    mulps xmm6, xmm13
    addps xmm0, xmm6

    mulps xmm13, xmm15
    mulps xmm7, xmm13
    addps xmm0, xmm7

    mulps xmm13, xmm15
    mulps xmm8, xmm13
    addps xmm0, xmm8

    // xmm0 - e^r

    // c = e^(n*ln(2) + r)
    // c = (2^n)*(e^r)
.Lfin:
    mulps xmm0, xmm14
    ret




