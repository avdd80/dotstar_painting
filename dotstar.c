#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
#include "dotstar.h"

/*                      3 bit Header   5 bit brightness level */
#define LED_BRIGHTNESS (0xE0         | 1                    )
#define SPI_SPEED_MHZ  ((unsigned long int)4)
#define DELAY_MS       4


unsigned int bmpwidth  = 350;
unsigned int bmpheight = 144;

/* Max size = 2048 x 144 pixels */
unsigned char framed_spi_data[2048][584];

/* 32 bit image data: 0x00RRGGBB */
extern const unsigned long image32[];
unsigned long              image32_transpose[2048*144];
extern const unsigned char clear_dotstar[584];
//unsigned long image32[4] = {0x00FF0000, 0x0000FF00, 0x000000FF, 0x0020AB50};

int main (void)
{

	unsigned int i;

	init ();

	for (i = 0; i < bmpwidth; i++)
	{
		flush_column (&framed_spi_data[i][0]);
		delay (DELAY_MS);
		/* delay 30.86 ms - 10 mph */
		/* data write overhead = 389 us */		
	}

	return 0;
}

/* Initialize data and clear LED strip */
void init (void)
{

	image_transpose ();

	prepare_frame (image32_transpose);
	
	wiringPiSPISetup (0, (SPI_SPEED_MHZ * 1000000));
	
	/* Clear the DotStar LED strip */
	flush_column ((unsigned char *)clear_dotstar);
	
	if (bmpheight != 144)
	{
		printf ("Exiting... Image height is %d. Consider resizing to 144", bmpheight);
		exit (1);
	}
	return;
	
}

/* This function expects a linear array of size N x 144 pixels (R, G, B) with 32 bits per pixel */
void prepare_frame (const unsigned long *image_ptr)
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

		for (j = 0; j < bmpheight; j++)
		{
			/* Add 1 to account for START OF FRAME marker */
			spi_pixel_index = 4 + j * 4;
			
			framed_spi_data[i][spi_pixel_index]     = LED_BRIGHTNESS;
			/* RED */
			framed_spi_data[i][spi_pixel_index + 3] = (unsigned char)((image_ptr[image_pixel_index] & 0xFF0000) >> 16);
			/* GREEN */
			framed_spi_data[i][spi_pixel_index + 2] = (unsigned char)((image_ptr[image_pixel_index] & 0x00FF00) >> 8);
			/* BLUE */
			framed_spi_data[i][spi_pixel_index + 1] = (unsigned char)((image_ptr[image_pixel_index] & 0x0000FF));

			/* Increment by 1 32 bit pixel for R, G, B values */
			image_pixel_index += 1;
			
			//printf ("%d\n", framed_spi_data[i][spi_pixel_index + 3]);
		}

		/* END OF FRAME marker */
		framed_spi_data[i][580] = 0xFF;
		framed_spi_data[i][581] = 0xFF;
		framed_spi_data[i][582] = 0xFF;
		framed_spi_data[i][583] = 0xFF;

	}
	
	//for (j = 0; j < 584; j++)
	//{
		//printf ("%d\n", framed_spi_data[0][j]);
	//}

	
	return;
}

void flush_column (unsigned char* data_ptr)
{

	wiringPiSPIDataRW (0, data_ptr, 584);
	return;
}

void image_transpose (void)
{
	unsigned int i, j;

	for (i = 0; i < bmpwidth; i++)
	{
		for (j = 0; j < bmpheight; j++)
		{
			image32_transpose[i*bmpheight + j] = image32[i + bmpwidth*j];
		}
	}
	return;
}