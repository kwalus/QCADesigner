#ifndef _CAD_UTIL_H_
#define _CAD_UTIL_H_

/* General-purpose function to scale one rectangle until it is inscribed in another rectangle */
void fit_rect_inside_rect (double dWidth, double dHeight, double *px, double *py, double *pdRectWidth, double *pdRectHeight) ;

#endif /* _CAD_UTIL_H_ */
