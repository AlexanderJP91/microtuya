#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <stdlib.h>

#define DEBUG

typedef enum result {   OK                  = 0,
						INPUT_FAIL          = 1,
						CONNECTION_ERROR    = 2,
						TX_ERROR            = 3,
						MAX_RX_ERROR        = 4,
                        RESPONSE_SIZE_ERROR = 5,
                        RESPONSE_DATA_ERROR = 6,
                        CRC_ERROR           = 7
			} result_t;

typedef struct rgb {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb_t;

typedef struct hsv {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv_t;

hsv_t rgb2hsv(rgb_t in);
rgb_t hsv2rgb(hsv_t in);

#endif // _TOOLS_H_