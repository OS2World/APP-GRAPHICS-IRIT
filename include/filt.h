/* filt.h: definitions for filter data types and routines */

#ifndef FILT_HDR
#define FILT_HDR

typedef double (*filt_func_type)(double, char *);
typedef void (*filt_print_func_type)(char *);
typedef void (*filt_init_func_type)(double, double, char *);

typedef struct Filt {		/* A 1-D FILTER */
    char *name;			/* name of filter */
    filt_func_type func;	/* filter function */
    double supp;		/* radius of nonzero portion */
    double blur;		/* blur factor (1=normal) */
    char windowme;		/* should filter be windowed? */
    char cardinal;		/* is this filter cardinal?
				   ie, does func(x) = (x==0) for integer x? */
    char unitrange;		/* does filter stay within the range [0..1] */
    filt_init_func_type initproc; /* initialize client data, if any */
    filt_print_func_type printproc; /* print client data, if any */
    char *clientdata;		/* client info to be passed to func */
} Filt;

#define filt_func(f, x) ((f)->func)((x), (f)->clientdata)
#define filt_print_client(f) ((f)->printproc)((f)->clientdata)

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

Filt *filt_find(char *name);
void filt_print(Filt *f);
Filt *filt_window(Filt *f, char *windowname);
void filt_catalog(void);

/* the filter collection: */

double filt_box(double x, char *d);      /* box, pulse, Fourier window, */
double filt_triangle(double x, char *d); /* triangle, Bartlett window, */
double filt_quadratic(double x, char *d);/* 3rd order (quadratic) b-spline */
double filt_cubic(double x, char *d);    /* 4th order (cubic) b-spline */
double filt_catrom(double x, char *d);   /* Catmull-Rom spl, Overhauser spl */
double filt_gaussian(double x, char *d); /* Gaussian (infinite) */
double filt_sinc(double x, char *d);     /* Sinc, perfect lowpass filter */
double filt_bessel(double x, char *d);   /* Bessel */
double filt_mitchell(double x, char *d); /* Mitchell & Netravali's cubic */
double filt_hanning(double x, char *d);  /* Hanning window */
double filt_hamming(double x, char *d);  /* Hamming window */
double filt_blackman(double x, char *d); /* Blackman window */
double filt_kaiser(double x, char *d);   /* parameterized Kaiser window */
double filt_normal(double x, char *d);   /* normal distribution (infinite) */

/* support routines */
double bessel_i0(double x);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* FILT_HDR */
