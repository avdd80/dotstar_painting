#include <stdio.h>
#include <wiringPiSPI.h>
#include "dotstar.h"

/*                      3 bit Header   5 bit brightness level */
#define LED_BRIGHTNESS (0xE0         | 7                    )
unsigned int bmpwidth  = 1633;
unsigned int bmpheight = 144;

/* Max size = 2048 x 144 pixels */
unsigned char framed_spi_data[2048][584];

/* 32 bit image data: 0x00RRGGBB */
extern const unsigned long image32[];

int main (void)
{

	unsigned int i;

	if (bmpheight != 144)
	{
		printf ("Exit. Image heigh is %d. Consider resizing to 144", bmpheight);
		exit (1);
	}

	void prepare_frame (image32);

	wiringPiSPISetup (0, 12000000);

	for (i = 0; i < bmpwidth; i++)
	{
		flush_column (&framed_spi_data[i][0]);

		/* delay 30.86 ms - 10 mph */
		/* data write overhead = 389 us */		
	}


	return 0;

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

		for (j = 0; j < 144; j++)
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
}

void flush_column (unsigned char* data_ptr)
{

	wiringPiSPIDataRW (0, data_ptr, 584);

}
