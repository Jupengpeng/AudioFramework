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
/* Generated on Sun Jul  2 14:18:50 EDT 2006 */

#include "codelet-rdft.h"

#ifdef HAVE_FMA

/* Generated by: ../../../genfft/gen_r2hc -fma -reorder-insns -schedule-for-pipeline -compact -variables 4 -pipeline-latency 4 -n 10 -name r2hc_10 -include r2hc.h */

/*
 * This function contains 34 FP additions, 14 FP multiplications,
 * (or, 24 additions, 4 multiplications, 10 fused multiply/add),
 * 29 stack variables, and 20 memory accesses
 */
/*
 * Generator Id's : 
 * $Id: algsimp.ml,v 1.9 2006-02-12 23:34:12 athena Exp $
 * $Id: fft.ml,v 1.4 2006-01-05 03:04:27 stevenj Exp $
 * $Id: gen_r2hc.ml,v 1.18 2006-02-12 23:34:12 athena Exp $
 */

#include "r2hc.h"

static void r2hc_10(const R *I, R *ro, R *io, stride is, stride ros, stride ios, INT v, INT ivs, INT ovs)
{
     DK(KP559016994, +0.559016994374947424102293417182819058860154590);
     DK(KP250000000, +0.250000000000000000000000000000000000000000000);
     DK(KP618033988, +0.618033988749894848204586834365638117720309180);
     DK(KP951056516, +0.951056516295153572116439333379382143405698634);
     INT i;
     for (i = v; i > 0; i = i - 1, I = I + ivs, ro = ro + ovs, io = io + ovs, MAKE_VOLATILE_STRIDE(is), MAKE_VOLATILE_STRIDE(ros), MAKE_VOLATILE_STRIDE(ios)) {
	  E Tt, T3, T7, Tq, T6, Tv, Tp, Tm, Th, T8, T1, T2, T9, Tr;
	  T1 = I[0];
	  T2 = I[WS(is, 5)];
	  {
	       E Te, Tn, Td, Tf, Tb, Tc;
	       Tb = I[WS(is, 4)];
	       Tc = I[WS(is, 9)];
	       Te = I[WS(is, 6)];
	       Tt = T1 + T2;
	       T3 = T1 - T2;
	       Tn = Tb + Tc;
	       Td = Tb - Tc;
	       Tf = I[WS(is, 1)];
	       {
		    E T4, T5, To, Tg;
		    T4 = I[WS(is, 2)];
		    T5 = I[WS(is, 7)];
		    T7 = I[WS(is, 8)];
		    To = Te + Tf;
		    Tg = Te - Tf;
		    Tq = T4 + T5;
		    T6 = T4 - T5;
		    Tv = Tn + To;
		    Tp = Tn - To;
		    Tm = Tg - Td;
		    Th = Td + Tg;
		    T8 = I[WS(is, 3)];
	       }
	  }
	  T9 = T7 - T8;
	  Tr = T7 + T8;
	  {
	       E Ty, Tk, Tx, Tj, Tu, Ts;
	       Tu = Tq + Tr;
	       Ts = Tq - Tr;
	       {
		    E Ta, Tl, Tw, Ti;
		    Ta = T6 + T9;
		    Tl = T6 - T9;
		    io[WS(ios, 4)] = KP951056516 * (FMA(KP618033988, Tp, Ts));
		    io[WS(ios, 2)] = KP951056516 * (FNMS(KP618033988, Ts, Tp));
		    Ty = Tu - Tv;
		    Tw = Tu + Tv;
		    io[WS(ios, 3)] = KP951056516 * (FMA(KP618033988, Tl, Tm));
		    io[WS(ios, 1)] = -(KP951056516 * (FNMS(KP618033988, Tm, Tl)));
		    Tk = Ta - Th;
		    Ti = Ta + Th;
		    ro[0] = Tt + Tw;
		    Tx = FNMS(KP250000000, Tw, Tt);
		    ro[WS(ros, 5)] = T3 + Ti;
		    Tj = FNMS(KP250000000, Ti, T3);
	       }
	       ro[WS(ros, 4)] = FMA(KP559016994, Ty, Tx);
	       ro[WS(ros, 2)] = FNMS(KP559016994, Ty, Tx);
	       ro[WS(ros, 3)] = FNMS(KP559016994, Tk, Tj);
	       ro[WS(ros, 1)] = FMA(KP559016994, Tk, Tj);
	  }
     }
}

static const kr2hc_desc desc = { 10, "r2hc_10", {24, 4, 10, 0}, &GENUS, 0, 0, 0, 0, 0 };

void X(codelet_r2hc_10) (planner *p) {
     X(kr2hc_register) (p, r2hc_10, &desc);
}

#else				/* HAVE_FMA */

/* Generated by: ../../../genfft/gen_r2hc -compact -variables 4 -pipeline-latency 4 -n 10 -name r2hc_10 -include r2hc.h */

/*
 * This function contains 34 FP additions, 12 FP multiplications,
 * (or, 28 additions, 6 multiplications, 6 fused multiply/add),
 * 26 stack variables, and 20 memory accesses
 */
/*
 * Generator Id's : 
 * $Id: algsimp.ml,v 1.9 2006-02-12 23:34:12 athena Exp $
 * $Id: fft.ml,v 1.4 2006-01-05 03:04:27 stevenj Exp $
 * $Id: gen_r2hc.ml,v 1.18 2006-02-12 23:34:12 athena Exp $
 */

#include "r2hc.h"

static void r2hc_10(const R *I, R *ro, R *io, stride is, stride ros, stride ios, INT v, INT ivs, INT ovs)
{
     DK(KP250000000, +0.250000000000000000000000000000000000000000000);
     DK(KP559016994, +0.559016994374947424102293417182819058860154590);
     DK(KP951056516, +0.951056516295153572116439333379382143405698634);
     DK(KP587785252, +0.587785252292473129168705954639072768597652438);
     INT i;
     for (i = v; i > 0; i = i - 1, I = I + ivs, ro = ro + ovs, io = io + ovs, MAKE_VOLATILE_STRIDE(is), MAKE_VOLATILE_STRIDE(ros), MAKE_VOLATILE_STRIDE(ios)) {
	  E Ti, Tt, Ta, Tn, Td, To, Te, Tv, T3, Tq, T6, Tr, T7, Tu, Tg;
	  E Th;
	  Tg = I[0];
	  Th = I[WS(is, 5)];
	  Ti = Tg - Th;
	  Tt = Tg + Th;
	  {
	       E T8, T9, Tb, Tc;
	       T8 = I[WS(is, 4)];
	       T9 = I[WS(is, 9)];
	       Ta = T8 - T9;
	       Tn = T8 + T9;
	       Tb = I[WS(is, 6)];
	       Tc = I[WS(is, 1)];
	       Td = Tb - Tc;
	       To = Tb + Tc;
	  }
	  Te = Ta + Td;
	  Tv = Tn + To;
	  {
	       E T1, T2, T4, T5;
	       T1 = I[WS(is, 2)];
	       T2 = I[WS(is, 7)];
	       T3 = T1 - T2;
	       Tq = T1 + T2;
	       T4 = I[WS(is, 8)];
	       T5 = I[WS(is, 3)];
	       T6 = T4 - T5;
	       Tr = T4 + T5;
	  }
	  T7 = T3 + T6;
	  Tu = Tq + Tr;
	  {
	       E Tl, Tm, Tf, Tj, Tk;
	       Tl = Td - Ta;
	       Tm = T3 - T6;
	       io[WS(ios, 1)] = FNMS(KP951056516, Tm, KP587785252 * Tl);
	       io[WS(ios, 3)] = FMA(KP587785252, Tm, KP951056516 * Tl);
	       Tf = KP559016994 * (T7 - Te);
	       Tj = T7 + Te;
	       Tk = FNMS(KP250000000, Tj, Ti);
	       ro[WS(ros, 1)] = Tf + Tk;
	       ro[WS(ros, 5)] = Ti + Tj;
	       ro[WS(ros, 3)] = Tk - Tf;
	  }
	  {
	       E Tp, Ts, Ty, Tw, Tx;
	       Tp = Tn - To;
	       Ts = Tq - Tr;
	       io[WS(ios, 2)] = FNMS(KP587785252, Ts, KP951056516 * Tp);
	       io[WS(ios, 4)] = FMA(KP951056516, Ts, KP587785252 * Tp);
	       Ty = KP559016994 * (Tu - Tv);
	       Tw = Tu + Tv;
	       Tx = FNMS(KP250000000, Tw, Tt);
	       ro[WS(ros, 2)] = Tx - Ty;
	       ro[0] = Tt + Tw;
	       ro[WS(ros, 4)] = Ty + Tx;
	  }
     }
}

static const kr2hc_desc desc = { 10, "r2hc_10", {28, 6, 6, 0}, &GENUS, 0, 0, 0, 0, 0 };

void X(codelet_r2hc_10) (planner *p) {
     X(kr2hc_register) (p, r2hc_10, &desc);
}

#endif				/* HAVE_FMA */
