/*
 * filt: package of 1-d signal filters, both FIR and IIR
 *
 * Paul Heckbert, ph@miro.berkeley.edu	23 Oct 1986, 10 Sept 1988
 *
 * Copyright (c) 1989  Paul S. Heckbert
 * This source may be used for peaceful, nonprofit purposes only, unless
 * under licence from the author. This notice should remain in the source.
 *
 *
 * Documentation:
 *  To use these routines,
 *	#include "filt.h"
 *  Then call filt_find to select the desired filter, e.g.
 *	Filt *f;
 *	f = filt_find("catrom");
 *  This filter function (impulse response) is then evaluated by calling
 *  filt_func(f, x).  Each filter is nonzero between -f->supp and f->supp.
 *  Typically one will use the filter something like this:
 *	double phase, x, weight;
 *	for (x=phase-f->supp; x<f->supp; x+=deltax) {
 *	    weight = filt_func(f, x);
 *	    # do something with weight
 *	}
 *
 *  Example of windowing an IIR filter:
 *	f = filt_find("sinc");
 *	# if you wanted to vary sinc width, set f->supp = desired_width here
 *	# e.g. f->supp = 6;
 *	f = filt_window(f, "blackman");
 *  You can then use f as above.
 */

/*
 * references:
 *
 * A.V. Oppenheim, R.W. Schafer, Digital Signal Processing, Prentice-Hall, 1975
 *
 * R.W. Hamming, Digital Filters, Prentice-Hall, Englewood Cliffs, NJ, 1983
 *
 * W.K. Pratt, Digital Image Processing, John Wiley and Sons, 1978
 *
 * H.S. Hou, H.C. Andrews, "Cubic Splines for Image Interpolation and
 *	Digital Filtering", IEEE Trans. Acoustics, Speech, and Signal Proc.,
 *	vol. ASSP-26, no. 6, Dec. 1978, pp. 508-517
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef AMIGA
extern double j1(double);
#endif

#include "irit_sm.h"
#include "misc_lib.h"
#include "filt.h"
#define ALLOC(ptr, type, n)  ptr = (type *) malloc((n)*sizeof(type))
#define str_eq               0 == strcmp

typedef struct mitchell_data {	/* data for parameterized Mitchell filter */
    double p0, p2, p3;
    double q0, q1, q2, q3;
} mitchell_data;

typedef struct kaiser_data{	/* data for parameterized Kaiser window */
    double a;		        /* = w*(N-1)/2 in Oppenheim&Schafer notation */
    double i0a;
    /*
     * typically 4<a<9
     * param a trades off main lobe width (sharpness)
     * for side lobe amplitude (ringing)
     */
} kaiser_data;

typedef struct window_data {   /* data for windowed function compound filter */
    Filt *filter;
    Filt *window;
} window_data;

static void filt_init(void);
static double window_func(double x, char *d);
static void window_print(char *d);
static void mitchell_init(double b, double c, char *d);
static void mitchell_print(char *d);
static void kaiser_init(double a, double b, char *d);
static void kaiser_print(char *d);

IRIT_STATIC_DATA mitchell_data md;
IRIT_STATIC_DATA kaiser_data kd;

#define NFILTMAX 30
IRIT_STATIC_DATA int nfilt = 0;

/*
 * note: for the IIR (infinite impulse response) filters,
 * gaussian, sinc, etc, the support values given are arbitrary,
 * convenient cutoff points, while for the FIR (finite impulse response)
 * filters the support is finite and absolute.
 */

IRIT_STATIC_DATA Filt filt[NFILTMAX] = {
/*  NAME	FUNC		SUPP	BLUR WIN CARD UNIT  OPT... */
   {"point",	filt_box,	0.0,	1.0,  0,  1,  1},
   {"box",	filt_box,       0.5,	1.0,  0,  1,  1},
   {"triangle",	filt_triangle,  1.0,	1.0,  0,  1,  1},
   {"quadratic",filt_quadratic, 1.5,	1.0,  0,  0,  1},
   {"cubic",	filt_cubic,     2.0,	1.0,  0,  0,  1},

   {"catrom",	filt_catrom,    2.0,	1.0,  0,  1,  0},
   {"mitchell",	filt_mitchell,	2.0,	1.0,  0,  0,  0,
	mitchell_init, mitchell_print, (char *)&md},

   {"gaussian",	filt_gaussian,  1.25,	1.0,  0,  0,  1},
   {"sinc",	filt_sinc,      4.0,	1.0,  1,  1,  0},
   {"bessel",	filt_bessel,    3.2383,	1.0,  1,  0,  0},

   {"hanning",	filt_hanning,	1.0,	1.0,  0,  1,  1},
   {"hamming",	filt_hamming,	1.0,	1.0,  0,  1,  1},
   {"blackman",	filt_blackman,	1.0,	1.0,  0,  1,  1},
   {"kaiser",	filt_kaiser,	1.0,	1.0,  0,  1,  1,
	kaiser_init, kaiser_print, (char *)&kd},
   {0}
};

/*-------------------- general filter routines --------------------*/

static void filt_init(void)
{
    /* count the filters to set nfilt */
    for (nfilt=0; nfilt<NFILTMAX && filt[nfilt].name; nfilt++);
    /* better way?? */
    mitchell_init(1./3., 1./3., (char *)&md);
    kaiser_init(6.5, 0., (char *)&kd);
}

/*
 * filt_find: return ptr to filter descriptor given filter name
 */

Filt *filt_find(char *name)
{
    int i;

    if (nfilt==0) filt_init();
    for (i=0; i<nfilt; i++)
	if (str_eq(name, filt[i].name))
	    return &filt[i];
    return 0;
}

/*
 * filt_insert: insert filter f in filter collection
 */

void filt_insert(Filt *f)
{
    if (nfilt==0) filt_init();
    if (filt_find(f->name)!=0) {
	IRIT_WARNING_MSG_PRINTF("filt_insert: there's already a filter called %s\n",
				f->name);
	return;
    }
    if (nfilt>=NFILTMAX) {
	IRIT_WARNING_MSG_PRINTF("filt_insert: too many filters: %d\n",
				nfilt+1);
	return;
    }
    filt[nfilt++] = *f;
}

/*
 * filt_catalog: print a filter catalog to stdout
 */

void filt_catalog(void)
{
    int i;

    if (nfilt==0) filt_init();
    for (i=0; i<nfilt; i++)
	filt_print(&filt[i]);
}

/*
 * filt_print: print info about a filter to stdout
 */

void filt_print(Filt *f)
{
    IRIT_INFO_MSG_PRINTF("%-9s\t%4.2f%s", f->name, f->supp,
			 f->windowme ? " (windowed by default)" : "");
    if (f->printproc) {
	IRIT_INFO_MSG("\n    ");
	filt_print_client(f);
    }
    IRIT_INFO_MSG("\n");
}

/*-------------------- windowing a filter --------------------*/

/*
 * filt_window: given an IIR filter f and the name of a window function,
 * create a compound filter that is the product of the two:
 * wf->func(x) = f->func(x) * w->func(x/s)
 *
 * note: allocates memory that is (probably) never freed
 */

Filt *filt_window(Filt *f, char *windowname)
{
    Filt *w, *wf;
    window_data *d;

    if (str_eq(windowname, "box")) return f;	/* window with box is NOP */
    w = filt_find(windowname);
    ALLOC(wf, Filt, 1);
    *wf = *f;
    ALLOC(wf->name, char, 50);
    sprintf(wf->name, "%s*%s", f->name, w->name);
    wf->func = window_func;
    if (f->printproc || w->printproc) wf->printproc = window_print;
    else wf->printproc = 0;
    ALLOC(d, window_data, 1);
    d->filter = f;
    d->window = w;
    wf->clientdata = (char *)d;
    return wf;
}

static double window_func(double x, char *d)
{
    register window_data *w;

    w = (window_data *)d;
#   ifdef DEBUG_FILT
    IRIT_INFO_MSG_PRINTF("%s*%s(%g) = %g*%g = %g\n",
	w->filter->name, w->window->name, x);
	filt_func(w->filter, x), filt_func(w->window, x/w->filter->supp),
	filt_func(w->filter, x) * filt_func(w->window, x/w->filter->supp));
#   endif
    return filt_func(w->filter, x) * filt_func(w->window, x/w->filter->supp);
}

static void window_print(char *d)
{
    window_data *w;

    w = (window_data *)d;
    if (w->filter->printproc) filt_print_client(w->filter);
    if (w->window->printproc) filt_print_client(w->window);
}

/*--------------- unit-area filters for unit-spaced samples ---------------*/

/* all filters centered on 0 */

double filt_box(double x, char *d)	/* box, pulse, Fourier window, */
{
    if (x<-.5) return 0.;
    if (x<.5) return 1.;
    return 0.;
}

double filt_triangle(double x, char *d)	/* triangle, Bartlett window, */
{
    if (x<-1.) return 0.;
    if (x<0.) return 1.+x;
    if (x<1.) return 1.-x;
    return 0.;
}

double filt_quadratic(double x, char *d) /* 3rd order (quadratic) b-spline */
{
    double t;

    if (x<-1.5) return 0.;
    if (x<-.5) {t = x+1.5; return .5*t*t;}
    if (x<.5) return .75-x*x;
    if (x<1.5) {t = x-1.5; return .5*t*t;}
    return 0.;
}

double filt_cubic(double x, char *d)	/* 4th order (cubic) b-spline */
{
    double t;

    if (x<-2.) return 0.;
    if (x<-1.) {t = 2.+x; return t*t*t/6.;}
    if (x<0.) return (4.+x*x*(-6.+x*-3.))/6.;
    if (x<1.) return (4.+x*x*(-6.+x*3.))/6.;
    if (x<2.) {t = 2.-x; return t*t*t/6.;}
    return 0.;
}

double filt_catrom(double x, char *d)/* Catmull-Rom spline, Overhauser spline */
{
    if (x<-2.) return 0.;
    if (x<-1.) return .5*(4.+x*(8.+x*(5.+x)));
    if (x<0.) return .5*(2.+x*x*(-5.+x*-3.));
    if (x<1.) return .5*(2.+x*x*(-5.+x*3.));
    if (x<2.) return .5*(4.+x*(-8.+x*(5.-x)));
    return 0.;
}

double filt_gaussian(double x, char *d)	/* Gaussian (infinite) */
{
    return exp(-2.*x*x)*sqrt(2./M_PI);
}

double filt_sinc(double x, char *d)/* Sinc, perfect lowpass filter (infinite) */
{
    return x==0. ? 1. : sin(M_PI*x)/(M_PI*x);
}

double filt_bessel(double x, char *d)/* Bessel (for circularly symm. 2-d filt, inf)*/
{
    /*
     * See Pratt "Digital Image Processing" p. 97 for Bessel functions
     * zeros are at approx x=1.2197, 2.2331, 3.2383, 4.2411, 5.2428, 6.2439,
     * 7.2448, 8.2454
     */
#if defined(OS2GCC) || defined (__WINNT__) || defined (__WINCE__)
    IRIT_WARNING_MSG("Filter \"bessel\" is not supported\n");
    return 0.; /* No j1 function in math library. */
#else
    return x==0. ? M_PI/4. : j1(M_PI*x)/(2.*x);
#endif
}

/*-------------------- parameterized filters --------------------*/

double filt_mitchell(double x, char *d)	/* Mitchell & Netravali's two-param cubic */
{
    register mitchell_data *m;

    /*
     * see Mitchell&Netravali, "Reconstruction Filters in Computer Graphics",
     * SIGGRAPH 88
     */
    m = (mitchell_data *)d;
    if (x<-2.) return 0.;
    if (x<-1.) return m->q0-x*(m->q1-x*(m->q2-x*m->q3));
    if (x<0.) return m->p0+x*x*(m->p2-x*m->p3);
    if (x<1.) return m->p0+x*x*(m->p2+x*m->p3);
    if (x<2.) return m->q0+x*(m->q1+x*(m->q2+x*m->q3));
    return 0.;
}

static void mitchell_init(double b, double c, char *d)
{
    mitchell_data *m;

    m = (mitchell_data *)d;
    m->p0 = (  6. -  2.*b        ) / 6.;
    m->p2 = (-18. + 12.*b +  6.*c) / 6.;
    m->p3 = ( 12. -  9.*b -  6.*c) / 6.;
    m->q0 = (	     8.*b + 24.*c) / 6.;
    m->q1 = (	  - 12.*b - 48.*c) / 6.;
    m->q2 = (	     6.*b + 30.*c) / 6.;
    m->q3 = (     -     b -  6.*c) / 6.;
}

static void mitchell_print(char *d)
{
    mitchell_data *m;

    m = (mitchell_data *)d;
    IRIT_INFO_MSG_PRINTF("mitchell: p0=%g p2=%g p3=%g q0=%g q1=%g q2=%g q3=%g\n",
	m->p0, m->p2, m->p3, m->q0, m->q1, m->q2, m->q3);
}

/*-------------------- window functions --------------------*/

double filt_hanning(double x, char *d)	/* Hanning window */
{
    return .5+.5*cos(M_PI*x);
}

double filt_hamming(double x, char *d)	/* Hamming window */
{
    return .54+.46*cos(M_PI*x);
}

double filt_blackman(double x, char *d)	/* Blackman window */
{
    return .42+.50*cos(M_PI*x)+.08*cos(2.*M_PI*x);
}

/*-------------------- parameterized windows --------------------*/

double filt_kaiser(double x, char *d)	/* parameterized Kaiser window */
{
    /* from Oppenheim & Schafer, Hamming */
    kaiser_data *k;

    k = (kaiser_data *)d;
    return bessel_i0(k->a*sqrt(1.-x*x))*k->i0a;
}

static void kaiser_init(double a, double b, char *d)
{
    kaiser_data *k;

    k = (kaiser_data *)d;
    k->a = a;
    k->i0a = 1./bessel_i0(a);
}

static void kaiser_print(char *d)
{
    kaiser_data *k;

    k = (kaiser_data *)d;
    IRIT_INFO_MSG_PRINTF("kaiser: a=%g i0a=%g\n", k->a, k->i0a);
}

double bessel_i0(double x)
{
    /*
     * modified zeroth order Bessel function of the first kind.
     * Are there better ways to compute this than the power series?
     */
    register int i;
    double sum, y, t;

    sum = 1.;
    y = x*x/4.;
    t = y;
    for (i=2; t>IRIT_EPS; i++) {
	sum += t;
	t *= (double)y/(i*i);
    }
    return sum;
}

/*--------------- filters for non-unit spaced samples ---------------*/

double filt_normal(double x, char *d)	/* normal distribution (infinite) */
{
    /*
     * normal distribution: has unit area, but it's not for unit spaced samples
     * Normal(x) = Gaussian(x/2)/2
     */
    return exp(-x*x/2.)/sqrt(2.*M_PI);
}
