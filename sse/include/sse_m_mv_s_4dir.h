#define _inline_sse_mult_su3_mat_vec_sum_4dir(aa,bb0,bb1,bb2,bb3,cc) \
{ \
__asm__ __volatile__ ("movlps %0, %%xmm0 \n\t" \
                      "movlps %1, %%xmm1 \n\t" \
                      "movlps %2, %%xmm2 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm0 \n\t" \
                      "shufps $0x44, %%xmm1, %%xmm1 \n\t" \
                      "shufps $0x44, %%xmm2, %%xmm2 \n\t" \
                      "movss %3, %%xmm3 \n\t" \
                      "movss %4, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm3 \n\t" \
                      "movss %5, %%xmm4" \
                      : \
                      : \
                      "m" ((bb0)->c[0]), \
                      "m" ((bb0)->c[1]), \
                      "m" ((bb0)->c[2]), \
                      "m" ((aa)[0].e[0][0].real), \
                      "m" ((aa)[0].e[1][0].real), \
                      "m" ((aa)[0].e[0][1].real)); \
__asm__ __volatile__ ("movss %0, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm4 \n\t" \
                      "mulps %%xmm0, %%xmm3 \n\t" \
                      "mulps %%xmm1, %%xmm4 \n\t" \
                      "addps %%xmm4, %%xmm3 \n\t" \
                      "movss %1, %%xmm5 \n\t" \
                      "movss %2, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm2, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm1 \n\t" \
                      "movss %3, %%xmm7 \n\t" \
                      "movss %4, %%xmm6 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm6 \n\t" \
                      "mulps %%xmm1, %%xmm6 \n\t" \
                      "shufps $0xB1, %%xmm0, %%xmm0 \n\t" \
                      "xorps %5, %%xmm0" \
                      : \
                      : \
                      "m" ((aa)[0].e[1][1].real), \
                      "m" ((aa)[0].e[0][2].real), \
                      "m" ((aa)[0].e[1][2].real), \
                      "m" ((aa)[0].e[2][0].real), \
                      "m" ((aa)[0].e[2][1].real), \
                      "m" (_sse_sgn13)); \
__asm__ __volatile__ ("shufps $0x11, %%xmm1, %%xmm1 \n\t" \
                      "xorps %0, %%xmm1 \n\t" \
                      "shufps $0xB1, %%xmm2, %%xmm2 \n\t" \
                      "xorps %1, %%xmm2 \n\t" \
                      "movss %2, %%xmm4 \n\t" \
                      "movss %3, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm4 \n\t" \
                      "mulps %%xmm0, %%xmm4 \n\t" \
                      "addps %%xmm4, %%xmm3 \n\t" \
                      "movss %4, %%xmm5 \n\t" \
                      "movss %5, %%xmm7" \
                      : \
                      : \
                      "m" (_sse_sgn13), \
                      "m" (_sse_sgn13), \
                      "m" ((aa)[0].e[0][0].imag), \
                      "m" ((aa)[0].e[1][0].imag), \
                      "m" ((aa)[0].e[0][1].imag), \
                      "m" ((aa)[0].e[1][1].imag)); \
__asm__ __volatile__ ("shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm1, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      "movss %0, %%xmm5 \n\t" \
                      "movss %1, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm2, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      : \
                      : \
                      "m" ((aa)[0].e[0][2].imag), \
                      "m" ((aa)[0].e[1][2].imag)); \
__asm__ __volatile__ ("movups %%xmm3, %0 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm1 \n\t" \
                      : \
                      "=m" ((cc)->c[0])); \
__asm__ __volatile__ ("movss %0, %%xmm7 \n\t" \
                      "movss %1, %%xmm5 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm1, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm6 \n\t" \
                      "shufps $0xB4, %%xmm2, %%xmm2 \n\t" \
                      "xorps %2, %%xmm2 \n\t" \
                      "movlps %3, %%xmm7 \n\t" \
                      "shufps $0x05, %%xmm7, %%xmm7 \n\t" \
                      "mulps %%xmm2, %%xmm7 \n\t" \
                      "addps %%xmm7, %%xmm6 \n\t" \
                      "movaps %%xmm6, %%xmm7 \n\t" \
                      "shufps $0xEE, %%xmm7, %%xmm7 \n\t" \
                      "addps %%xmm7, %%xmm6 \n\t" \
                      : \
                      : \
                      "m" ((aa)[0].e[2][0].imag), \
                      "m" ((aa)[0].e[2][1].imag), \
                      "m" (_sse_sgn4), \
                      "m" ((aa)[0].e[2][2])); \
__asm__ __volatile__ ("movlps %%xmm6, %0 \n\t" \
                      : \
                      "=m" ((cc)->c[2])); \
__asm__ __volatile__ ("movlps %0, %%xmm0 \n\t" \
                      "movlps %1, %%xmm1 \n\t" \
                      "movlps %2, %%xmm2 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm0 \n\t" \
                      "shufps $0x44, %%xmm1, %%xmm1 \n\t" \
                      "shufps $0x44, %%xmm2, %%xmm2 \n\t" \
                      "movss %3, %%xmm3 \n\t" \
                      "movss %4, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm3 \n\t" \
                      "movss %5, %%xmm4" \
                      : \
                      : \
                      "m" ((bb1)->c[0]), \
                      "m" ((bb1)->c[1]), \
                      "m" ((bb1)->c[2]), \
                      "m" ((aa)[1].e[0][0].real), \
                      "m" ((aa)[1].e[1][0].real), \
                      "m" ((aa)[1].e[0][1].real)); \
__asm__ __volatile__ ("movss %0, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm4 \n\t" \
                      "mulps %%xmm0, %%xmm3 \n\t" \
                      "mulps %%xmm1, %%xmm4 \n\t" \
                      "addps %%xmm4, %%xmm3 \n\t" \
                      "movss %1, %%xmm5 \n\t" \
                      "movss %2, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm2, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm1 \n\t" \
                      "movss %3, %%xmm7 \n\t" \
                      "movss %4, %%xmm6 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm6 \n\t" \
                      "mulps %%xmm1, %%xmm6 \n\t" \
                      "shufps $0xB1, %%xmm0, %%xmm0 \n\t" \
                      "xorps %5, %%xmm0" \
                      : \
                      : \
                      "m" ((aa)[1].e[1][1].real), \
                      "m" ((aa)[1].e[0][2].real), \
                      "m" ((aa)[1].e[1][2].real), \
                      "m" ((aa)[1].e[2][0].real), \
                      "m" ((aa)[1].e[2][1].real), \
                      "m" (_sse_sgn13)); \
__asm__ __volatile__ ("shufps $0x11, %%xmm1, %%xmm1 \n\t" \
                      "xorps %0, %%xmm1 \n\t" \
                      "shufps $0xB1, %%xmm2, %%xmm2 \n\t" \
                      "xorps %1, %%xmm2 \n\t" \
                      "movss %2, %%xmm4 \n\t" \
                      "movss %3, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm4 \n\t" \
                      "mulps %%xmm0, %%xmm4 \n\t" \
                      "addps %%xmm4, %%xmm3 \n\t" \
                      "movss %4, %%xmm5 \n\t" \
                      "movss %5, %%xmm7" \
                      : \
                      : \
                      "m" (_sse_sgn13), \
                      "m" (_sse_sgn13), \
                      "m" ((aa)[1].e[0][0].imag), \
                      "m" ((aa)[1].e[1][0].imag), \
                      "m" ((aa)[1].e[0][1].imag), \
                      "m" ((aa)[1].e[1][1].imag)); \
__asm__ __volatile__ ("shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm1, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      "movss %0, %%xmm5 \n\t" \
                      "movss %1, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm2, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      "movups %2, %%xmm7 \n\t" \
                      "addps %%xmm3, %%xmm7 \n\t" \
                      : \
                      : \
                      "m" ((aa)[1].e[0][2].imag), \
                      "m" ((aa)[1].e[1][2].imag), \
                      "m" ((cc)->c[0])); \
__asm__ __volatile__ ("movups %%xmm7, %0 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm1 \n\t" \
                      : \
                      "=m" ((cc)->c[0])); \
__asm__ __volatile__ ("movss %0, %%xmm7 \n\t" \
                      "movss %1, %%xmm5 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm1, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm6 \n\t" \
                      "shufps $0xB4, %%xmm2, %%xmm2 \n\t" \
                      "xorps %2, %%xmm2 \n\t" \
                      "movlps %3, %%xmm7 \n\t" \
                      "shufps $0x05, %%xmm7, %%xmm7 \n\t" \
                      "mulps %%xmm2, %%xmm7 \n\t" \
                      "addps %%xmm7, %%xmm6 \n\t" \
                      "movaps %%xmm6, %%xmm7 \n\t" \
                      "shufps $0xEE, %%xmm7, %%xmm7 \n\t" \
                      "addps %%xmm7, %%xmm6 \n\t" \
                      "movlps %4, %%xmm7 \n\t" \
                      "addps %%xmm6, %%xmm7 \n\t" \
                      : \
                      : \
                      "m" ((aa)[1].e[2][0].imag), \
                      "m" ((aa)[1].e[2][1].imag), \
                      "m" (_sse_sgn4), \
                      "m" ((aa)[1].e[2][2]), \
                      "m" ((cc)->c[2])); \
__asm__ __volatile__ ("movlps %%xmm7, %0 \n\t" \
                      : \
                      "=m" ((cc)->c[2])); \
__asm__ __volatile__ ("movlps %0, %%xmm0 \n\t" \
                      "movlps %1, %%xmm1 \n\t" \
                      "movlps %2, %%xmm2 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm0 \n\t" \
                      "shufps $0x44, %%xmm1, %%xmm1 \n\t" \
                      "shufps $0x44, %%xmm2, %%xmm2 \n\t" \
                      "movss %3, %%xmm3 \n\t" \
                      "movss %4, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm3 \n\t" \
                      "movss %5, %%xmm4" \
                      : \
                      : \
                      "m" ((bb2)->c[0]), \
                      "m" ((bb2)->c[1]), \
                      "m" ((bb2)->c[2]), \
                      "m" ((aa)[2].e[0][0].real), \
                      "m" ((aa)[2].e[1][0].real), \
                      "m" ((aa)[2].e[0][1].real)); \
__asm__ __volatile__ ("movss %0, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm4 \n\t" \
                      "mulps %%xmm0, %%xmm3 \n\t" \
                      "mulps %%xmm1, %%xmm4 \n\t" \
                      "addps %%xmm4, %%xmm3 \n\t" \
                      "movss %1, %%xmm5 \n\t" \
                      "movss %2, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm2, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm1 \n\t" \
                      "movss %3, %%xmm7 \n\t" \
                      "movss %4, %%xmm6 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm6 \n\t" \
                      "mulps %%xmm1, %%xmm6 \n\t" \
                      "shufps $0xB1, %%xmm0, %%xmm0 \n\t" \
                      "xorps %5, %%xmm0" \
                      : \
                      : \
                      "m" ((aa)[2].e[1][1].real), \
                      "m" ((aa)[2].e[0][2].real), \
                      "m" ((aa)[2].e[1][2].real), \
                      "m" ((aa)[2].e[2][0].real), \
                      "m" ((aa)[2].e[2][1].real), \
                      "m" (_sse_sgn13)); \
__asm__ __volatile__ ("shufps $0x11, %%xmm1, %%xmm1 \n\t" \
                      "xorps %0, %%xmm1 \n\t" \
                      "shufps $0xB1, %%xmm2, %%xmm2 \n\t" \
                      "xorps %1, %%xmm2 \n\t" \
                      "movss %2, %%xmm4 \n\t" \
                      "movss %3, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm4 \n\t" \
                      "mulps %%xmm0, %%xmm4 \n\t" \
                      "addps %%xmm4, %%xmm3 \n\t" \
                      "movss %4, %%xmm5 \n\t" \
                      "movss %5, %%xmm7" \
                      : \
                      : \
                      "m" (_sse_sgn13), \
                      "m" (_sse_sgn13), \
                      "m" ((aa)[2].e[0][0].imag), \
                      "m" ((aa)[2].e[1][0].imag), \
                      "m" ((aa)[2].e[0][1].imag), \
                      "m" ((aa)[2].e[1][1].imag)); \
__asm__ __volatile__ ("shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm1, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      "movss %0, %%xmm5 \n\t" \
                      "movss %1, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm2, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      "movups %2, %%xmm7 \n\t" \
                      "addps %%xmm3, %%xmm7 \n\t" \
                      : \
                      : \
                      "m" ((aa)[2].e[0][2].imag), \
                      "m" ((aa)[2].e[1][2].imag), \
                      "m" ((cc)->c[0])); \
__asm__ __volatile__ ("movups %%xmm7, %0 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm1 \n\t" \
                      : \
                      "=m" ((cc)->c[0])); \
__asm__ __volatile__ ("movss %0, %%xmm7 \n\t" \
                      "movss %1, %%xmm5 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm1, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm6 \n\t" \
                      "shufps $0xB4, %%xmm2, %%xmm2 \n\t" \
                      "xorps %2, %%xmm2 \n\t" \
                      "movlps %3, %%xmm7 \n\t" \
                      "shufps $0x05, %%xmm7, %%xmm7 \n\t" \
                      "mulps %%xmm2, %%xmm7 \n\t" \
                      "addps %%xmm7, %%xmm6 \n\t" \
                      "movaps %%xmm6, %%xmm7 \n\t" \
                      "shufps $0xEE, %%xmm7, %%xmm7 \n\t" \
                      "addps %%xmm7, %%xmm6 \n\t" \
                      "movlps %4, %%xmm7 \n\t" \
                      "addps %%xmm6, %%xmm7 \n\t" \
                      : \
                      : \
                      "m" ((aa)[2].e[2][0].imag), \
                      "m" ((aa)[2].e[2][1].imag), \
                      "m" (_sse_sgn4), \
                      "m" ((aa)[2].e[2][2]), \
                      "m" ((cc)->c[2])); \
__asm__ __volatile__ ("movlps %%xmm7, %0 \n\t" \
                      : \
                      "=m" ((cc)->c[2])); \
__asm__ __volatile__ ("movlps %0, %%xmm0 \n\t" \
                      "movlps %1, %%xmm1 \n\t" \
                      "movlps %2, %%xmm2 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm0 \n\t" \
                      "shufps $0x44, %%xmm1, %%xmm1 \n\t" \
                      "shufps $0x44, %%xmm2, %%xmm2 \n\t" \
                      "movss %3, %%xmm3 \n\t" \
                      "movss %4, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm3 \n\t" \
                      "movss %5, %%xmm4" \
                      : \
                      : \
                      "m" ((bb3)->c[0]), \
                      "m" ((bb3)->c[1]), \
                      "m" ((bb3)->c[2]), \
                      "m" ((aa)[3].e[0][0].real), \
                      "m" ((aa)[3].e[1][0].real), \
                      "m" ((aa)[3].e[0][1].real)); \
__asm__ __volatile__ ("movss %0, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm4 \n\t" \
                      "mulps %%xmm0, %%xmm3 \n\t" \
                      "mulps %%xmm1, %%xmm4 \n\t" \
                      "addps %%xmm4, %%xmm3 \n\t" \
                      "movss %1, %%xmm5 \n\t" \
                      "movss %2, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm2, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm1 \n\t" \
                      "movss %3, %%xmm7 \n\t" \
                      "movss %4, %%xmm6 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm6 \n\t" \
                      "mulps %%xmm1, %%xmm6 \n\t" \
                      "shufps $0xB1, %%xmm0, %%xmm0 \n\t" \
                      "xorps %5, %%xmm0" \
                      : \
                      : \
                      "m" ((aa)[3].e[1][1].real), \
                      "m" ((aa)[3].e[0][2].real), \
                      "m" ((aa)[3].e[1][2].real), \
                      "m" ((aa)[3].e[2][0].real), \
                      "m" ((aa)[3].e[2][1].real), \
                      "m" (_sse_sgn13)); \
__asm__ __volatile__ ("shufps $0x11, %%xmm1, %%xmm1 \n\t" \
                      "xorps %0, %%xmm1 \n\t" \
                      "shufps $0xB1, %%xmm2, %%xmm2 \n\t" \
                      "xorps %1, %%xmm2 \n\t" \
                      "movss %2, %%xmm4 \n\t" \
                      "movss %3, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm4 \n\t" \
                      "mulps %%xmm0, %%xmm4 \n\t" \
                      "addps %%xmm4, %%xmm3 \n\t" \
                      "movss %4, %%xmm5 \n\t" \
                      "movss %5, %%xmm7" \
                      : \
                      : \
                      "m" (_sse_sgn13), \
                      "m" (_sse_sgn13), \
                      "m" ((aa)[3].e[0][0].imag), \
                      "m" ((aa)[3].e[1][0].imag), \
                      "m" ((aa)[3].e[0][1].imag), \
                      "m" ((aa)[3].e[1][1].imag)); \
__asm__ __volatile__ ("shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm1, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      "movss %0, %%xmm5 \n\t" \
                      "movss %1, %%xmm7 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm2, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm3 \n\t" \
                      "movups %2, %%xmm7 \n\t" \
                      "addps %%xmm3, %%xmm7 \n\t" \
                      : \
                      : \
                      "m" ((aa)[3].e[0][2].imag), \
                      "m" ((aa)[3].e[1][2].imag), \
                      "m" ((cc)->c[0])); \
__asm__ __volatile__ ("movups %%xmm7, %0 \n\t" \
                      "shufps $0x44, %%xmm0, %%xmm1 \n\t" \
                      : \
                      "=m" ((cc)->c[0])); \
__asm__ __volatile__ ("movss %0, %%xmm7 \n\t" \
                      "movss %1, %%xmm5 \n\t" \
                      "shufps $0x00, %%xmm7, %%xmm5 \n\t" \
                      "mulps %%xmm1, %%xmm5 \n\t" \
                      "addps %%xmm5, %%xmm6 \n\t" \
                      "shufps $0xB4, %%xmm2, %%xmm2 \n\t" \
                      "xorps %2, %%xmm2 \n\t" \
                      "movlps %3, %%xmm7 \n\t" \
                      "shufps $0x05, %%xmm7, %%xmm7 \n\t" \
                      "mulps %%xmm2, %%xmm7 \n\t" \
                      "addps %%xmm7, %%xmm6 \n\t" \
                      "movaps %%xmm6, %%xmm7 \n\t" \
                      "shufps $0xEE, %%xmm7, %%xmm7 \n\t" \
                      "addps %%xmm7, %%xmm6 \n\t" \
                      "movlps %4, %%xmm7 \n\t" \
                      "addps %%xmm6, %%xmm7 \n\t" \
                      : \
                      : \
                      "m" ((aa)[3].e[2][0].imag), \
                      "m" ((aa)[3].e[2][1].imag), \
                      "m" (_sse_sgn4), \
                      "m" ((aa)[3].e[2][2]), \
                      "m" ((cc)->c[2])); \
__asm__ __volatile__ ("movlps %%xmm7, %0 \n\t" \
                      : \
                      "=m" ((cc)->c[2])); \
}
