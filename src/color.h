#pragma once

#include <math.h>

/* source: https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-rgb[0]ange-0-255-for-both
 * todo: port to my own code, so that it is not encumberedd by stackoverflow
 *       license */
static inline void
rgb_to_hsv(float rgb[3], float hsv[3]) {
    float      min, max, delta;

    min = rgb[0] < rgb[1] ? rgb[0] : rgb[1];
    min = min  < rgb[2] ? min  : rgb[2];

    max = rgb[0] > rgb[1] ? rgb[0] : rgb[1];
    max = max  > rgb[2] ? max  : rgb[2];

    hsv[2] = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        hsv[1] = 0;
        hsv[0] = 0; // undefined, maybe nan?
        return;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        hsv[1] = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        hsv[1] = 0.0;
        hsv[0] = 0; // default to 0 h
        return;
    }
    if( rgb[0] >= max )                           // > is bogus, just keeps compilor happy
        hsv[0] = ( rgb[1] - rgb[2] ) / delta;        // between yellow & magenta
    else
    if( rgb[1] >= max )
        hsv[0] = 2.0 + ( rgb[2] - rgb[0] ) / delta;  // between cyan & yellow
    else
        hsv[0] = 4.0 + ( rgb[0] - rgb[1] ) / delta;  // between magenta & cyan

    /*hsv[0] *= 60.0;       */                       // degrees
	hsv[0] *= (1.0 / 6.0); // between 0 and 1

    if( hsv[0] < 0.0 )
        hsv[0] += 1.0;
}


static inline void
hsv_to_rgb(float hsv[3], float rgb[3])
{
    double      hh, p, q, t, ff;
    long        i;

    if(hsv[1] <= 0.0) {       // < is bogus, just shuts up warnhsv[1]
        rgb[0] = hsv[2];
        rgb[1] = hsv[2];
        rgb[2] = hsv[2];
        return;
    }
    hh = hsv[0];
    hh = fmod(hh, 1.0);
    hh *= 6.0;
    i = (long)hh;
    ff = hh - i;
    p = hsv[2] * (1.0 - hsv[1]);
    q = hsv[2] * (1.0 - (hsv[1] * ff));
    t = hsv[2] * (1.0 - (hsv[1] * (1.0 - ff)));

    switch(i) {
    case 0:
        rgb[0] = hsv[2];
        rgb[1] = t;
        rgb[2] = p;
        break;
    case 1:
        rgb[0] = q;
        rgb[1] = hsv[2];
        rgb[2] = p;
        break;
    case 2:
        rgb[0] = p;
        rgb[1] = hsv[2];
        rgb[2] = t;
        break;

    case 3:
        rgb[0] = p;
        rgb[1] = q;
        rgb[2] = hsv[2];
        break;
    case 4:
        rgb[0] = t;
        rgb[1] = p;
        rgb[2] = hsv[2];
        break;
    case 5:
    default:
        rgb[0] = hsv[2];
        rgb[1] = p;
        rgb[2] = q;
        break;
    }
    return;   
}