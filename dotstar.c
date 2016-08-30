#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
#include "dotstar.h"

/*                      3 bit Header   5 bit brightness level */
#define LED_BRIGHTNESS (0xE0         | 7                    )
#define SPI_SPEED_MHZ  ((unsigned long int)4)

unsigned int bmpwidth  = 1;
unsigned int bmpheight = 144;

/* Max size = 2048 x 144 pixels */
unsigned char framed_spi_data[2048][584];

/* 32 bit image data: 0x00RRGGBB */
//extern const unsigned long image32[];
extern const unsigned char clear_dotstar[584];
unsigned long image322[4] = {0x00FF0000, 0x0000FF00, 0x000000FF, 0x0020AB50};

int main (void)
{

	unsigned int i;

	init ();

	for (i = 0; i < bmpwidth; i++)
	{
		flush_column (&framed_spi_data[i][0]);

		/* delay 30.86 ms - 10 mph */
		/* data write overhead = 389 us */		
	}

	return 0;
}

/* Initialize data and clear LED strip */
void init (void)
{

	prepare_frame (image322);
	
	wiringPiSPISetup (0, (SPI_SPEED_MHZ * 1000000));
	
	/* Clear the DotStar LED strip */
	flush_column ((unsigned char *)clear_dotstar);
	
	exit (1);
	
	if (bmpheight != 144)
	{
		printf ("Exiting... Image heigh is %d. Consider resizing to 144", bmpheight);
		exit (1);
	}
	
}

/* This function expects a linear array of size N x 144 pixels (R, G, B) with 32 bits per pixel */
void prepare_frame (unsigned long *image_ptr)
{
	unsigned int i, j;
	unsigned int image_pixel_index = 0;
	unsigned int spi_pixel_index   = 0;

	for (i = 0; i < bmpwidth; i++)
	{
		/* START OF FRAME marker */
		framed_spi_data[i][0] = 0;
		framed_spi_data[i][1] = 0;
		framed_spi_data[i][2] = 0;
		framed_spi_data[i][3] = 0;

		for (j = 0; j < 1; j++)
		{
			/* Add 1 to account for START OF FRAME marker */
			spi_pixel_index = 1 + j * 4;
			
			framed_spi_data[i][spi_pixel_index]     = LED_BRIGHTNESS;
			framed_spi_data[i][spi_pixel_index + 1] = image_ptr[image_pixel_index];
			framed_spi_data[i][spi_pixel_index + 2] = image_ptr[image_pixel_index];
			framed_spi_data[i][spi_pixel_index + 3] = image_ptr[image_pixel_index];
			
			
			/* Increment by 1 32 bit pixel for R, G, B values */
			image_pixel_index += 1;
		}

		/* END OF FRAME marker */
		framed_spi_data[i][580] = 0xFF;
		framed_spi_data[i][581] = 0xFF;
		framed_spi_data[i][582] = 0xFF;
		framed_spi_data[i][583] = 0xFF;

	}
	return;
}

void flush_column (unsigned char* data_ptr)
{

	wiringPiSPIDataRW (0, data_ptr, 584);
	return;
}
