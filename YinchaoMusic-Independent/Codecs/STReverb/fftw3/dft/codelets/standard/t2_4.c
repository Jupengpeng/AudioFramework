/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Sat Jul  1 14:08:21 EDT 2006 */

#include "codelet-dft.h"

#ifdef HAVE_FMA

/* Generated by: ../../../genfft/gen_twiddle -fma -reorder-insns -schedule-for-pipeline -compact -variables 4 -pipeline-latency 4 -twiddle-log3 -precompute-twiddles -n 4 -name t2_4 -include t.h */

/*
 * This function contains 24 FP additions, 16 FP multiplications,
 * (or, 16 additions, 8 multiplications, 8 fused multiply/add),
 * 33 stack variables, and 16 memory accesses
 */
/*
 * Generator Id's : 
 * $Id: algsimp.ml,v 1.9 2006-02-12 23:34:12 athena Exp $
 * $Id: fft.ml,v 1.4 2006-01-05 03:04:27 stevenj Exp $
 * $Id: gen_twiddle.ml,v 1.24 2006-02-12 23:34:12 athena Exp $
 */

#include "t.h"

static const R *t2_4(R *ri, R *ii, const R *W, stride ios, INT m, INT dist)
{
     INT i;
     for (i = m; i > 0; i = i - 1, ri = ri + dist, ii = ii + dist, W = W + 4, MAKE_VOLATILE_STRIDE(ios)) {
	  E Ti, Tq, To, Te, Ty, Tz, Tm, Ts;
	  {
	       E T2, T6, T3, T5;
	       T2 = W[0];
	       T6 = W[3];
	       T3 = W[2];
	       T5 = W[1];
	       {
		    E T1, Tx, Td, Tw, Tj, Tl, Ta, T4, Tk, Tr;
		    T1 = ri[0];
		    Ta = T2 * T6;
		    T4 = T2 * T3;
		    Tx = ii[0];
		    {
			 E T8, Tb, T7, Tc;
			 T8 = ri[WS(ios, 2)];
			 Tb = FNMS(T5, T3, Ta);
			 T7 = FMA(T5, T6, T4);
			 Tc = ii[WS(ios, 2)];
			 {
			      E Tf, Th, T9, Tv, Tg, Tp;
			      Tf = ri[WS(ios, 1)];
			      Th = ii[WS(ios, 1)];
			      T9 = T7 * T8;
			      Tv = T7 * Tc;
			      Tg = T2 * Tf;
			      Tp = T2 * Th;
			      Td = FMA(Tb, Tc, T9);
			      Tw = FNMS(Tb, T8, Tv);
			      Ti = FMA(T5, Th, Tg);
			      Tq = FNMS(T5, Tf, Tp);
			 }
			 Tj = ri[WS(ios, 3)];
			 Tl = ii[WS(ios, 3)];
		    }
		    To = T1 - Td;
		    Te = T1 + Td;
		    Ty = Tw + Tx;
		    Tz = Tx - Tw;
		    Tk = T3 * Tj;
		    Tr = T3 * Tl;
		    Tm = FMA(T6, Tl, Tk);
		    Ts = FNMS(T6, Tj, Tr);
	       }
	  }
	  {
	       E Tn, TA, Tu, Tt;
	       Tn = Ti + Tm;
	       TA = Ti - Tm;
	       Tu = Tq + Ts;
	       Tt = Tq - Ts;
	       ii[WS(ios, 3)] = TA + Tz;
	       ii[WS(ios, 1)] = Tz - TA;
	       ri[0] = Te + Tn;
	       ri[WS(ios, 2)] = Te - Tn;
	       ri[WS(ios, 1)] = To + Tt;
	       ri[WS(ios, 3)] = To - Tt;
	       ii[WS(ios, 2)] = Ty - Tu;
	       ii[0] = Tu + Ty;
	  }
     }
     return W;
}

static const tw_instr twinstr[] = {
     {TW_CEXP, 0, 1},
     {TW_CEXP, 0, 3},
     {TW_NEXT, 1, 0}
};

static const ct_desc desc = { 4, "t2_4", twinstr, &GENUS, {16, 8, 8, 0}, 0, 0, 0 };

void X(codelet_t2_4) (planner *p) {
     X(kdft_dit_register) (p, t2_4, &desc);
}
#else				/* HAVE_FMA */

/* Generated by: ../../../genfft/gen_twiddle -compact -variables 4 -pipeline-latency 4 -twiddle-log3 -precompute-twiddles -n 4 -name t2_4 -include t.h */

/*
 * This function contains 24 FP additions, 16 FP multiplications,
 * (or, 16 additions, 8 multiplications, 8 fused multiply/add),
 * 21 stack variables, and 16 memory accesses
 */
/*
 * Generator Id's : 
 * $Id: algsimp.ml,v 1.9 2006-02-12 23:34:12 athena Exp $
 * $Id: fft.ml,v 1.4 2006-01-05 03:04:27 stevenj Exp $
 * $Id: gen_twiddle.ml,v 1.24 2006-02-12 23:34:12 athena Exp $
 */

#include "t.h"

static const R *t2_4(R *ri, R *ii, const R *W, stride ios, INT m, INT dist)
{
     INT i;
     for (i = m; i > 0; i = i - 1, ri = ri + dist, ii = ii + dist, W = W + 4, MAKE_VOLATILE_STRIDE(ios)) {
	  E T2, T4, T3, T5, T6, T8;
	  T2 = W[0];
	  T4 = W[1];
	  T3 = W[2];
	  T5 = W[3];
	  T6 = FMA(T2, T3, T4 * T5);
	  T8 = FNMS(T4, T3, T2 * T5);
	  {
	       E T1, Tp, Ta, To, Te, Tk, Th, Tl, T7, T9;
	       T1 = ri[0];
	       Tp = ii[0];
	       T7 = ri[WS(ios, 2)];
	       T9 = ii[WS(ios, 2)];
	       Ta = FMA(T6, T7, T8 * T9);
	       To = FNMS(T8, T7, T6 * T9);
	       {
		    E Tc, Td, Tf, Tg;
		    Tc = ri[WS(ios, 1)];
		    Td = ii[WS(ios, 1)];
		    Te = FMA(T2, Tc, T4 * Td);
		    Tk = FNMS(T4, Tc, T2 * Td);
		    Tf = ri[WS(ios, 3)];
		    Tg = ii[WS(ios, 3)];
		    Th = FMA(T3, Tf, T5 * Tg);
		    Tl = FNMS(T5, Tf, T3 * Tg);
	       }
	       {
		    E Tb, Ti, Tn, Tq;
		    Tb = T1 + Ta;
		    Ti = Te + Th;
		    ri[WS(ios, 2)] = Tb - Ti;
		    ri[0] = Tb + Ti;
		    Tn = Tk + Tl;
		    Tq = To + Tp;
		    ii[0] = Tn + Tq;
		    ii[WS(ios, 2)] = Tq - Tn;
	       }
	       {
		    E Tj, Tm, Tr, Ts;
		    Tj = T1 - Ta;
		    Tm = Tk - Tl;
		    ri[WS(ios, 3)] = Tj - Tm;
		    ri[WS(ios, 1)] = Tj + Tm;
		    Tr = Tp - To;
		    Ts = Te - Th;
		    ii[WS(ios, 1)] = Tr - Ts;
		    ii[WS(ios, 3)] = Ts + Tr;
	       }
	  }
     }
     return W;
}

static const tw_instr twinstr[] = {
     {TW_CEXP, 0, 1},
     {TW_CEXP, 0, 3},
     {TW_NEXT, 1, 0}
};

static const ct_desc desc = { 4, "t2_4", twinstr, &GENUS, {16, 8, 8, 0}, 0, 0, 0 };

void X(codelet_t2_4) (planner *p) {
     X(kdft_dit_register) (p, t2_4, &desc);
}
#endif				/* HAVE_FMA */
