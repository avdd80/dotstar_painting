#ifndef _DOTSTAR_H
#define _DOTSTAR_H

/* Initialize data and clear LED strip */
void init (void);

/* This function expects a linear array of size N x 144 pixels (R, G, B) with 32 bits per pixel */
void prepare_frame (const unsigned long *image_ptr);

/* Flushes the column data to SPI */
void flush_column (unsigned char* data_ptr);

/* Perform a transpose on the image */
void image_transpose (void);

void load_image (void);

#endif /* _DOTSTAR_H */