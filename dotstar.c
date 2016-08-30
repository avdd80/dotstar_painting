#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
#include <time.h>
#include "dotstar.h"

/*                      3 bit Header   5 bit brightness level */
#define LED_BRIGHTNESS (0xE0         | 5                    )
#define SPI_SPEED_MHZ  ((unsigned long int)4)
#define DELAY_MS       10

#define BMPWIDTH        1633
#define BMPHEIGHT       144


unsigned int bmpwidth = BMPWIDTH;
unsigned int bmpheight = BMPHEIGHT;

/* Image loaded as 0xRR GG BB AA */
unsigned int image_ptr[BMPHEIGHT][BMPWIDTH];

/* Max size = 2048 x 144 pixels */
unsigned char framed_spi_data[2048][584];

unsigned int               transposed_image[BMPWIDTH][BMPHEIGHT];
extern const unsigned char clear_dotstar[584];

int main (void)
{

	unsigned int i;

	init ();
	
	printf ("init done\n");

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

	load_image ();

	image_transpose ();
	
	printf ("transpose done\n");

	prepare_frame (transposed_image);
	
	printf ("prepare frame done\n");
	
	wiringPiSPISetup (0, (SPI_SPEED_MHZ * 1000000));
	
	/* Clear the DotStar LED strip */
	flush_column ((unsigned char *)clear_dotstar);

	return;
	
}

/* This function expects a linear array of size N x 144 pixels (R, G, B) with 32 bits per pixel */
void prepare_frame (const unsigned long *data_ptr)
{
	unsigned int i, j;
	unsigned int image_pixel_index = 0;
	unsigned int spi_pixel_index   = 0;
	unsigned char r, g, b;
	unsigned int gamma;

	printf ("preparing frame\n");

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
			
			r = (unsigned char)((data_ptr[image_pixel_index] & 0x000000FF));
			g = (unsigned char)((data_ptr[image_pixel_index] & 0x0000FF00) >> 8);
			b = (unsigned char)((data_ptr[image_pixel_index] & 0x00FF0000) >> 16);
			
			gamma = r * 3 + g * 6 + b;
			
			gamma >>= 7;
			
			framed_spi_data[i][spi_pixel_index]     = 0xE0 | ((char)gamma);
			
			/* RED */
			framed_spi_data[i][spi_pixel_index + 3] = r;
			
			/* GREEN */
			framed_spi_data[i][spi_pixel_index + 2] = g; 
			/* BLUE */
			framed_spi_data[i][spi_pixel_index + 1] = b; 

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

void load_image (void)
{

	FILE* fp;
	unsigned int num_pixels, i;
	int bytes;
	int dummy;
	
	fp = fopen ("test.rgba", "rb");

	printf ("Reading image %dx%d\n", bmpwidth, bmpheight);
	
	if (bmpheight != 144)
	{
		printf ("Exiting... Image height is %d. Consider resizing to 144", bmpheight);
		exit (1);
	}
	
	num_pixels = bmpwidth * bmpheight;
	
	for (i = 0; i < bmpheight; i++)
	{
		fread ((unsigned char*)image_ptr[i], 1, bmpwidth, fp);
	}
	
	printf ("Image read done");
	
	fscanf (fp, "%d", dummy);
	printf ("dummy read done");
	
	fclose (fp);
	
	printf ("read bytes = %d\n", bytes);

	printf ("image_loaded\n");

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

	printf ("transpose\n");

	for (i = 0; i < bmpwidth; i++)
	{
		for (j = 0; j < bmpheight; j++)
		{
			transposed_image[i][j] = image_ptr[j][i];
		}
	}

	return;
}
