#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>// floor

/*
based on
c++ program from :
http://commons.wikimedia.org/wiki/File:Color_complex_plot.jpg
by      Claudio Rocchini
gcc d.c -lm -Wall
http://en.wikipedia.org/wiki/Domain_coloring
*/

const double


        PI = 3.1415926535897932384626433832795;

const double


        E = 2.7182818284590452353602874713527;


/*
complex domain coloring
Given a complex number z=re^{ i \theta},
hue represents the argument ( phase, theta ),
        sat and value represents the modulus
*/
int GiveHSV(double complex z, double HSVcolor[3]) {
    //The HSV, or HSB, model describes colors in terms of hue, saturation, and value (brightness).

    // hue = f(argument(z))
    //hue values range from .. to ..
    double a = carg(z); //
    while (a < 0) a += 2 * PI;
    a /= 2 * PI;
    // radius of z
    double m = cabs(z); //
    double ranges = 0;
    double rangee = 1;
    while (m > rangee) {
        ranges = rangee;
        rangee *= E;
    }
    double k = (m - ranges) / (rangee - ranges);
    // saturation = g(abs(z))
    double sat = k < 0.5 ? k * 2 : 1 - (k - 0.5) * 2;
    sat = 1 - pow((1 - sat), 3);
    sat = 0.4 + sat * 0.6;
    // value = h(abs(z))
    double val = k < 0.5 ? k * 2 : 1 - (k - 0.5) * 2;
    val = 1 - val;
    val = 1 - pow((1 - val), 3);
    val = 0.6 + val * 0.4;

    HSVcolor[0] = a;
    HSVcolor[1] = sat;
    HSVcolor[2] = val;
    return 0;
}


int GiveRGBfromHSV(double HSVcolor[3], unsigned char RGBcolor[3]) {
    double r, g, b;
    double h;
    double s;
    double v;
    h = HSVcolor[0]; // hue
    s = HSVcolor[1]; //  saturation;
    v = HSVcolor[2]; // = value;
    if (s == 0)
        r = g = b = v;
    else {
        if (h == 1) h = 0;
        double z = floor(h * 6);
        int i = (int) z;
        double f = (h * 6 - z);
        double p = v * (1 - s);
        double q = v * (1 - s * f);
        double t = v * (1 - s * (1 - f));
        switch (i) {
            case 0:
                r = v;
                g = t;
                b = p;
                break;
            case 1:
                r = q;
                g = v;
                b = p;
                break;
            case 2:
                r = p;
                g = v;
                b = t;
                break;
            case 3:
                r = p;
                g = q;
                b = v;
                break;
            case 4:
                r = t;
                g = p;
                b = v;
                break;
            case 5:
                r = v;
                g = p;
                b = q;
                break;
        }
    }
    int c;
    c = (int) (256 * r);
    if (c > 255) c = 255;
    RGBcolor[0] = c;
    c = (int) (256 * g);
    if (c > 255) c = 255;
    RGBcolor[1] = c;
    c = (int) (256 * b);
    if (c > 255) c = 255;
    RGBcolor[2] = c;
    return 0;
}


int GiveRGBColor(double complex z, unsigned char RGBcolor[3]) {
    static double HSVcolor[3];
    GiveHSV(z, HSVcolor);
    GiveRGBfromHSV(HSVcolor, RGBcolor);
    return 0;
}

//
double complex fun(double complex c) {
    return (cpow(c, 2) - 1) * cpow(c - 2.0 - I, 2) / (cpow(c, 2) + 2 + 2 * I);
} //

int main() {
    // screen (integer ) coordinate
    const int dimx = 800;
    const int dimy = 800;
    // world ( double) coordinate
    const double reMin = -2;
    const double reMax = 2;
    const double imMin = -2;
    const double imMax = 2;

    static unsigned char RGBcolor[3];
    FILE *fp;
    char *filename = "complex.ppm";
    fp = fopen(filename, "wb");
    fprintf(fp, "P6\n%d %d\n255\n", dimx, dimy);

    int i, j;
    for (j = 0; j < dimy; ++j) {
        double im = imMax - (imMax - imMin) * j / (dimy - 1);
        for (i = 0; i < dimx; ++i) {
            double re = reMax - (reMax - reMin) * i / (dimx - 1);
            double complex z = re + im * I; //
            double complex v = fun(z); //
            GiveRGBColor(v, RGBcolor);

            fwrite(RGBcolor, 1, 3, fp);
        }
    }
    fclose(fp);
    printf("OK - file %s saved\n", filename);
    return 0;
}