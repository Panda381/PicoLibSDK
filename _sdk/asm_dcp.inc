
// ****************************************************************************
//
//                        DCP Double-precision Coprocessor
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040

/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define apsr_nzcv r15

// === shortcuts to DCP instructions

.macro INIT
 cdp p4,#0,c0,c0,c0,#0
.endm

.macro ADD0
 cdp p4,#0,c0,c0,c1,#0
.endm

.macro ADD1
 cdp p4,#1,c0,c0,c1,#0
.endm

.macro SUB1
 cdp p4,#1,c0,c0,c1,#1
.endm

.macro SQR0
 cdp p4,#2,c0,c0,c1,#0
.endm

.macro NORM
 cdp p4,#8,c0,c0,c2,#0
.endm

.macro NRDF
 cdp p4,#8,c0,c0,c2,#1
.endm

.macro NRDD
 cdp p4,#8,c0,c0,c0,#1
.endm

.macro NTDC
 cdp p4,#8,c0,c0,c0,#2
.endm

.macro NRDC
 cdp p4,#8,c0,c0,c0,#3
.endm

.macro WXMD rl,rh
 mcrr p4,#0,\rl,\rh,c0
.endm

.macro WYMD rl,rh
 mcrr p4,#0,\rl,\rh,c1
.endm

.macro WEFD rl,rh
 mcrr p4,#0,\rl,\rh,c2
.endm

.macro WXUP rl,rh
 mcrr p4,#1,\rl,\rh,c0
.endm

.macro WYUP rl,rh
 mcrr p4,#1,\rl,\rh,c1
.endm

.macro WXYU rl,rh
 mcrr p4,#1,\rl,\rh,c2
.endm

.macro WXMS rl,rh
 mcrr p4,#2,\rl,\rh,c0
.endm

.macro WXMO rl,rh
 mcrr p4,#3,\rl,\rh,c0
.endm

.macro WXDD rl,rh
 mcrr p4,#4,\rl,\rh,c0
.endm

.macro WXDQ rl,rh
 mcrr p4,#5,\rl,\rh,c0
.endm

.macro WXUC rl,rh
 mcrr p4,#6,\rl,\rh,c0
.endm

.macro WXIC rl,rh
 mcrr p4,#7,\rl,\rh,c0
.endm

.macro WXDC rl,rh
 mcrr p4,#8,\rl,\rh,c0
.endm

.macro WXFC rl,rh
 mcrr p4,#9,\rl,\rh,c2
.endm

.macro WXFM rl,rh
 mcrr p4,#10,\rl,\rh,c0
.endm

.macro WXFD rl,rh
 mcrr p4,#11,\rl,\rh,c0
.endm

.macro WXFQ rl,rh
 mcrr p4,#12,\rl,\rh,c0
.endm

.macro RXVD rt
 mrc p4,#0,\rt,c0,c0,#0
.endm

.macro RCMP rt
 mrc p4,#0,\rt,c0,c0,#1
.endm

.macro RDFA rt
 mrc p4,#0,\rt,c0,c2,#0
.endm

.macro RDFS rt
 mrc p4,#0,\rt,c0,c2,#1
.endm

.macro RDFM rt
 mrc p4,#0,\rt,c0,c2,#2
.endm

.macro RDFD rt
 mrc p4,#0,\rt,c0,c2,#3
.endm

.macro RDFQ rt
 mrc p4,#0,\rt,c0,c2,#4
.endm

.macro RDFG rt
 mrc p4,#0,\rt,c0,c2,#5
.endm

.macro RDIC rt
 mrc p4,#0,\rt,c0,c3,#0
.endm

.macro RDUC rt
 mrc p4,#0,\rt,c0,c3,#1
.endm

.macro RXMD rl,rh
 mrrc p4,#0,\rl,\rh,c8
.endm

.macro RYMD rl,rh
 mrrc p4,#0,\rl,\rh,c9
.endm

.macro REFD rl,rh
 mrrc p4,#0,\rl,\rh,c10
.endm

.macro RXMS rl,rh,s
 mrrc p4,#\s,\rl,\rh,c4
.endm

.macro RYMS rl,rh,s
 mrrc p4,#\s,\rl,\rh,c5
.endm

.macro RXYH rl,rh
 mrrc p4,#1,\rl,\rh,c1
.endm

.macro RYMR rl,rh
 mrrc p4,#2,\rl,\rh,c1
.endm

.macro RXMQ rl,rh
 mrrc p4,#4,\rl,\rh,c1
.endm

.macro RDDA rl,rh
 mrrc p4,#1,\rl,\rh,c0
.endm

.macro RDDS rl,rh
 mrrc p4,#3,\rl,\rh,c0
.endm

.macro RDDM rl,rh
 mrrc p4,#5,\rl,\rh,c0
.endm

.macro RDDD rl,rh
 mrrc p4,#7,\rl,\rh,c0
.endm

.macro RDDQ rl,rh
 mrrc p4,#9,\rl,\rh,c0
.endm

.macro RDDG rl,rh
 mrrc p4,#11,\rl,\rh,c0
.endm

.macro PXVD rt
 mrc2 p4,#0,\rt,c0,c0,#0
.endm

.macro PCMP rt
 mrc2 p4,#0,\rt,c0,c0,#1
.endm

.macro PDFA rt
 mrc2 p4,#0,\rt,c0,c2,#0
.endm

.macro PDFS rt
 mrc2 p4,#0,\rt,c0,c2,#1
.endm

.macro PDFM rt
 mrc2 p4,#0,\rt,c0,c2,#2
.endm

.macro PDFD rt
 mrc2 p4,#0,\rt,c0,c2,#3
.endm

.macro PDFQ rt
 mrc2 p4,#0,\rt,c0,c2,#4
.endm

.macro PDFG rt
 mrc2 p4,#0,\rt,c0,c2,#5
.endm

.macro PDIC rt
 mrc2 p4,#0,\rt,c0,c3,#0
.endm

.macro PDUC rt
 mrc2 p4,#0,\rt,c0,c3,#1
.endm

.macro PXMD rl,rh
 mrrc2 p4,#0,\rl,\rh,c8
.endm

.macro PYMD rl,rh
 mrrc2 p4,#0,\rl,\rh,c9
.endm

.macro PEFD rl,rh
 mrrc2 p4,#0,\rl,\rh,c10
.endm

.macro PXMS rl,rh,s
 mrrc2 p4,#\s,\rl,\rh,c4
.endm

.macro PYMS rl,rh,s
 mrrc2 p4,#\s,\rl,\rh,c5
.endm

.macro PXYH rl,rh
 mrrc2 p4,#1,\rl,\rh,c1
.endm

.macro PYMR rl,rh
 mrrc2 p4,#2,\rl,\rh,c1
.endm

.macro PXMQ rl,rh
 mrrc2 p4,#4,\rl,\rh,c1
.endm

.macro PDDA rl,rh
 mrrc2 p4,#1,\rl,\rh,c0
.endm

.macro PDDS rl,rh
 mrrc2 p4,#3,\rl,\rh,c0
.endm

.macro PDDM rl,rh
 mrrc2 p4,#5,\rl,\rh,c0
.endm

.macro PDDD rl,rh
 mrrc2 p4,#7,\rl,\rh,c0
.endm

.macro PDDQ rl,rh
 mrrc2 p4,#9,\rl,\rh,c0
.endm

.macro PDDG rl,rh
 mrrc2 p4,#11,\rl,\rh,c0
.endm

// === Canned instruction sequences for use with the DCP

.macro dcp_fadd_m rz,rx,ry
  WXYU \rx,\ry
  ADD0
  ADD1
  NRDF
  RDFA \rz
.endm

.macro dcp_fsub_m rz,rx,ry
  WXYU \rx,\ry
  ADD0
  SUB1
  NRDF
  RDFS \rz
.endm

.macro dcp_fmul_m rz,rx,ry,ra,rb
  WXYU \rx,\ry
  RXYH \ra,\rb
  umull \ra,\rb,\ra,\rb
  WXFM \ra,\rb
  NRDF
  RDFM \rz
.endm

.macro dcp_fxprod_m rzl,rzh,rx,ry,ra,rb
  WXYU \rx,\ry
  RXYH \ra,\rb
  umull \ra,\rb,\ra,\rb
  WXMO \ra,\rb
  NRDD
  RDDM \rzl,\rzh
.endm

.macro dcp_fdiv_fast_m rz,rx,ry,ra,rb,rc
  WXYU \rx,\ry
  RYMR \ra,\rb
  umull \rb,\rc,\ra,\rb
  mvn \rc,\rc,lsl #2
  smmlar \ra,\rc,\ra,\ra
  smmulr \rc,\rc,\rc
  smmlar \ra,\rc,\ra,\ra
  RXYH \rb,\rc
  umull \ra,\rb,\ra,\rb
  WXFD \rb,\rb
  NRDF
  RDFD \rz
.endm

.macro dcp_fdiv_m rz,rx,ry,ra,rb,rc,rd
  WXYU \rx,\ry
  RYMR \ra,\rb
  umull \rb,\rc,\ra,\rb
  mvn \rc,\rc,lsl #2
  smmlar \ra,\rc,\ra,\ra
  smmulr \rc,\rc,\rc
  smmlar \ra,\rc,\ra,\ra
  RXYH \rb,\rc
  umull \rd,\ra,\ra,\rb
  orr \ra,\ra,\ra,lsr #24
  bic \ra,\ra,\ra,lsr #25
  bic \ra,\ra,#15
  mov \rc,\rc,lsr #7
  mul \rd,\ra,\rc
  rsb \rd,\rd,\rb,lsl #22
  sub \ra,\ra,\rd,lsr #31
  WXFD \ra,\ra
  NRDF
  RDFD \rz
.endm

.macro dcp_fsqrt_fast_m rz,rx,ra,rb,rc,rd
  WXYU \rx,\rx
  SQR0
  RXMQ \rc,\rd
  umull \ra,\rb,\rc,\rc
  umull \ra,\rb,\rb,\rd
  mov \rb,\rb,lsl #3
  sub \rb,\rb,#2147483648
  smmlsr \rc,\rc,\rb,\rc
  umull \ra,\rb,\rc,\rc
  umull \ra,\rb,\rb,\rd
  movs \ra,\ra,lsr #28
  adc \rb,\ra,\rb,lsl #4
  smmulr \ra,\rb,\rc
  sub \rc,\rc,\ra,asr #1
  umull \ra,\rb,\rc,\rd
  WXFQ \ra,\rb
  NRDF
  RDFQ \rz
.endm

.macro dcp_fsqrt_m rz,rx,ra,rb,rc,rd
  WXYU \rx,\rx
  SQR0
  RXMQ \rc,\rd
  umull \ra,\rb,\rc,\rc
  umull \ra,\rb,\rb,\rd
  mov \rb,\rb,lsl #3
  sub \rb,\rb,#2147483648
  smmlsr \rc,\rc,\rb,\rc
  umull \ra,\rb,\rc,\rc
  umull \ra,\rb,\rb,\rd
  movs \ra,\ra,lsr #28
  adc \rb,\ra,\rb,lsl #4
  smmulr \ra,\rb,\rc
  sub \rc,\rc,\ra,asr #1
  umull \ra,\rb,\rc,\rd
  orr \rb,\rb,#63
  mov \ra,\rb,lsr #5
  mul \ra,\ra,\ra
  rsb \ra,\ra,\rd,lsl #18
  bic \rb,\rb,\ra,lsr #26
  WXFQ \ra,\rb
  NRDF
  RDFQ \rz
.endm

.macro dcp_fclassify_m rz,rx
  WXYU \rx,\rx
  RXVD \rz
.endm

.macro dcp_fcmp_m rz,rx,ry
  WXYU \rx,\ry
  ADD0
  RCMP \rz
.endm

.macro dcp_dadd_m rzl,rzh,rxl,rxh,ryl,ryh
  WXUP \rxl,\rxh
  WYUP \ryl,\ryh
  ADD0
  ADD1
  NRDD
  RDDA \rzl,\rzh
.endm

.macro dcp_dsub_m rzl,rzh,rxl,rxh,ryl,ryh
  WXUP \rxl,\rxh
  WYUP \ryl,\ryh
  ADD0
  SUB1
  NRDD
  RDDS \rzl,\rzh
.endm

.macro dcp_dmul_m rzl,rzh,rxl,rxh,ryl,ryh,ra,rb,rc,rd,re,rf,rg
  WXUP \rxl,\rxh
  WYUP \ryl,\ryh
  RXMS \ra,\rb,0
  RYMS \rc,\rd,0
  umull \re,\rf,\ra,\rc
  movs \rg,#0
  umlal \rf,\rg,\ra,\rd
  umlal \rf,\rg,\rb,\rc
  WXMS \re,\rf
  movs \re,#0
  umlal \rg,\re,\rb,\rd
  WXMO \rg,\re
  NRDD
  RDDM \rzl,\rzh
.endm

.macro dcp_ddiv_fast_m rzl,rzh,rxl,rxh,ryl,ryh,ra,rb,rc,rd,re
  WXUP \rxl,\rxh
  WYUP \ryl,\ryh
  RYMR \ra,\rb
  umull \rb,\rc,\ra,\rb
  mvn \rc,\rc,lsl #2
  smmlar \ra,\rc,\ra,\ra
  smmulr \rc,\rc,\rc
  smmlar \ra,\rc,\ra,\ra
  sub \re,\ra,\ra,lsr #31
  RXMS \rc,\rd,0
  smmulr \rb,\re,\rd
  RYMS \rc,\rd,1
  umull \rc,\ra,\rb,\rc
  mla \ra,\rb,\rd,\ra
  RXMS \rc,\rd,4
  sub \ra,\rc,\ra
  smmulr \rc,\ra,\re
  mov \rd,\rb,lsr #4
  adds \ra,\rc,\rb,lsl #28
  adc \rb,\rd,\rc,asr #31
  WXDD \ra,\rb
  NRDD
  RDDD \rzl,\rzh
.endm

.macro dcp_ddiv_m rzl,rzh,rxl,rxh,ryl,ryh,ra,rb,rc,rd,re
  WXUP \rxl,\rxh
  WYUP \ryl,\ryh
  RYMR \ra,\rb
  umull \rb,\rc,\ra,\rb
  mvn \rc,\rc,lsl #2
  smmlar \ra,\rc,\ra,\ra
  smmulr \rc,\rc,\rc
  smmlar \ra,\rc,\ra,\ra
  sub \re,\ra,\ra,lsr #31
  RXMS \rc,\rd,0
  smmulr \rb,\re,\rd
  RYMS \rc,\rd,1
  umull \rc,\ra,\rb,\rc
  mla \ra,\rb,\rd,\ra
  RXMS \rc,\rd,4
  sub \ra,\rc,\ra
  smmulr \rc,\ra,\re
  mov \rd,\rb,lsr #4
  adds \ra,\rc,\rb,lsl #28
  adc \rb,\rd,\rc,asr #31
  orr \ra,\ra,\rb,lsr #21
  bic \ra,\ra,\rb,lsr #22
  bic \ra,\ra,#7
  RYMS \rc,\rd,7
  umull \rd,\re,\ra,\rc
  RYMS \rc,\rd,7
  mla \re,\ra,\rd,\re
  mla \re,\rb,\rc,\re
  RXMS \rc,\rd,0
  sub \re,\re,\rc,lsl #18
  orr \ra,\ra,\re,lsr #29
  sub \ra,\ra,#1
  WXDD \ra,\rb
  NRDD
  RDDD \rzl,\rzh
.endm

.macro dcp_dsqrt_fast_m rzl,rzh,rxl,rxh,ra,rb,rc,rd,re
  WXUP \rxl,\rxh
  SQR0
  RXMQ \rc,\rd
  umull \ra,\rb,\rc,\rc
  umull \ra,\rb,\rb,\rd
  mov \rb,\rb,lsl #3
  sub \rb,\rb,#2147483648
  smmlsr \rc,\rc,\rb,\rc
  umull \ra,\rb,\rc,\rc
  umull \ra,\rb,\rb,\rd
  movs \ra,\ra,lsr #28
  adc \rb,\ra,\rb,lsl #4
  smmulr \ra,\rb,\rc
  sub \rc,\rc,\ra,asr #1
  umull \ra,\rb,\rc,\rd
  mov \rb,\rb,lsr #1
  umull \rd,\ra,\rb,\rb
  RXMS \ra,\re,6
  sub \ra,\ra,\rd
  smmulr \ra,\ra,\rc
  add \rb,\rb,\ra,asr #28
  mov \ra,\ra,lsl #4
  WXDQ \ra,\rb
  NRDD
  RDDQ \rzl,\rzh
.endm

.macro dcp_dsqrt_m rzl,rzh,rxl,rxh,ra,rb,rc,rd,re
  WXUP \rxl,\rxh
  SQR0
  RXMQ \rc,\rd
  umull \ra,\rb,\rc,\rc
  umull \ra,\rb,\rb,\rd
  mov \rb,\rb,lsl #3
  sub \rb,\rb,#2147483648
  smmlsr \rc,\rc,\rb,\rc
  umull \ra,\rb,\rc,\rc
  umull \ra,\rb,\rb,\rd
  movs \ra,\ra,lsr #28
  adc \rb,\ra,\rb,lsl #4
  smmulr \ra,\rb,\rc
  sub \rc,\rc,\ra,asr #1
  umull \ra,\rb,\rc,\rd
  mov \rb,\rb,lsr #1
  umull \rd,\ra,\rb,\rb
  RXMS \ra,\re,6
  sub \rd,\ra,\rd
  smmulr \rd,\rd,\rc
  add \rb,\rb,\rd,asr #28
  mov \rd,\rd,lsl #4
  bic \rd,\rd,#255
  orr \rd,\rd,#128
  umull \re,\rc,\rd,\rd
  mul \re,\rb,\rb
  umlal \rc,\re,\rb,\rd
  umlal \rc,\re,\rb,\rd
  sub \re,\re,\ra
  orr \rd,\rd,\re,lsr #24
  sub \rd,\rd,#11
  WXDQ \rd,\rb
  NRDD
  RDDQ \rzl,\rzh
.endm

.macro dcp_dclassify_m rz,rxl,rxh
  WXUP \rxl,\rxh
  RXVD \rz
.endm

.macro dcp_dcmp_m rz,rxl,rxh,ryl,ryh
  WXUP \rxl,\rxh
  WYUP \ryl,\ryh
  ADD0
  RCMP \rz
.endm

.macro dcp_float2double_m rzl,rzh,rx
  WXYU \rx,\rx
  NRDD
  RDDG \rzl,\rzh
.endm

.macro dcp_double2float_m rz,rxl,rxh
  WXUP \rxl,\rxh
  NRDF
  RDFG \rz
.endm

.macro dcp_int2double_m rzl,rzh,rx
  WXIC \rx,\rx
  ADD0
  SUB1
  NRDD
  RDDS \rzl,\rzh
.endm

.macro dcp_uint2double_m rzl,rzh,rx
  WXUC \rx,\rx
  ADD0
  SUB1
  NRDD
  RDDS \rzl,\rzh
.endm

.macro dcp_int2float_m rz,rx
  WXIC \rx,\rx
  ADD0
  SUB1
  NRDF
  RDFS \rz
.endm

.macro dcp_uint2float_m rz,rx
  WXUC \rx,\rx
  ADD0
  SUB1
  NRDF
  RDFS \rz
.endm

.macro dcp_double2int_m rz,rxl,rxh
  WXDC \rxl,\rxh
  ADD0
  ADD1
  NTDC
  RDIC \rz
.endm

.macro dcp_double2uint_m rz,rxl,rxh
  WXDC \rxl,\rxh
  ADD0
  ADD1
  NTDC
  RDUC \rz
.endm

.macro dcp_float2int_m rz,rx
  WXFC \rx,\rx
  ADD0
  ADD1
  NTDC
  RDIC \rz
.endm

.macro dcp_float2uint_m rz,rx
  WXFC \rx,\rx
  ADD0
  ADD1
  NTDC
  RDUC \rz
.endm

.macro dcp_double2int_r_m rz,rxl,rxh
  WXDC \rxl,\rxh
  ADD0
  ADD1
  NRDC
  RDIC \rz
.endm

.macro dcp_double2uint_r_m rz,rxl,rxh
  WXDC \rxl,\rxh
  ADD0
  ADD1
  NRDC
  RDUC \rz
.endm

.macro dcp_float2int_r_m rz,rx
  WXFC \rx,\rx
  ADD0
  ADD1
  NRDC
  RDIC \rz
.endm

.macro dcp_float2uint_r_m rz,rx
  WXFC \rx,\rx
  ADD0
  ADD1
  NRDC
  RDUC \rz
.endm
