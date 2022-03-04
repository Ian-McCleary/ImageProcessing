#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <fcntl.h>

#define THREADS 4

#define FILTER_WIDTH 3
#define FILTER_HEIGHT 3

#define RGB_MAX 255

typedef struct {
	unsigned char r, g, b;
} PPMPixel;

typedef struct {
    int x, y;
    PPMPixel *data;
} PPMImage;


struct parameter {
	PPMPixel *image;         //original image
	PPMPixel *result;        //filtered image
	unsigned long int w;     //width of image
	unsigned long int h;     //height of image
	unsigned long int start; //starting point of work
	unsigned long int size;  //equal share of work (almost equal if odd)
};


/*This is the thread function. It will compute the new values for the region of image specified in params (start to start+size) using convolution.
    (1) For each pixel in the input image, the filter is conceptually placed on top ofthe image with its origin lying on that pixel.
    (2) The  values  of  each  input  image  pixel  under  the  mask  are  multiplied  by the corresponding filter values.
    (3) The results are summed together to yield a single output value that is placed in the output image at the location of the pixel being processed on the input.
 
 */
void *threadfn(void *params)
{
	
	int laplacian[FILTER_WIDTH][FILTER_HEIGHT] =
	{
	  -1, -1, -1,
	  -1,  8, -1,
	  -1, -1, -1,
	};

    int red, green, blue;
    
    /*For all pixels in the work region of image (from start to start+size)
      Multiply every value of the filter with corresponding image pixel. Note: this is NOT matrix multiplication.
     
     //truncate values smaller than zero and larger than 255
      Store the new values of r,g,b in p->result.
     */
		
	return NULL;
}


/*Create a new P6 file to save the filtered image in. Write the header block
 e.g. P6
      Width Height
      Max color value
 then write the image data.
 The name of the new file shall be "name" (the second argument).
 */
void writeImage(PPMPixel *image, char *name, unsigned long int width, unsigned long int height)
{

    
}

/* Open the filename image for reading, and parse it.
    Example of a ppm header:    //http://netpbm.sourceforge.net/doc/ppm.html
    P6                  -- image format
    # comment           -- comment lines begin with
    ## another comment  -- any number of comment lines
    200 300             -- image width & height
    255                 -- max color value
 
 Check if the image format is P6. If not, print invalid format error message.
 Read the image size information and store them in width and height.
 Check the rgb component, if not 255, display error message.
 Return: pointer to PPMPixel that has the pixel data of the input image (filename)
 */
PPMImage *readImage(const char *filename, unsigned long int *width, unsigned long int *height){


	PPMImage *img;
	char buff[16];
    int c, rgb_comp_color;
    FILE *fp;

	//read image format
    fp = fopen(filename, "rb");
    if (!fp){
        printf("Unable to open file %s\n", filename);
        exit(1);
    }

    //read image format
    if (!fgets(buff, sizeof(buff), fp)) {
        perror(filename);
        exit(1);
    }

	//check the image format by reading the first two characters in filename and compare them to P6.
    if (buff[0] != 'P' || buff[1] != '6'){
        printf("Invalid image format. Should be P6\n");
        exit(1);
    }

	

	
	//read image size information
	

	//Read rgb component. Check if it is equal to RGB_MAX. If  not, display error message.
	
    
    //allocate memory for img. NOTE: A ppm image of w=200 and h=300 will contain 60000 triplets (i.e. for r,g,b), ---> 18000 bytes.

    //allocate memory for entire image based on ppmimage struct
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img){
        printf("Memory allocation failed\n");
        exit(1);
    }
    //If there are comments in the file, skip them. You may assume that comments exist only in the header block.
    /*
    c = getc(fp);
    while (c == '#'){
        while (getc(fp) != '\n'){
            c = getc(fp);
        }
    } 

    ungetc(c, fp);
    */
   //skip comments
    c = getc(fp);
    if (c == '#'){
        printf("hash found\n");
        fscanf(fp, "%*[^\n]");
        //c = getc(fp);
    }
    //read image size information
    
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
        printf("Image size: %d, %d\n", img->x, img->y);
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        exit(1);
    }
    printf("width: %d  Height: %d\n", img->x, img->y);

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
         fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
         exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color!= 255) {
         fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
         exit(1);
    }

    while (fgetc(fp) != '\n') ;
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
         fprintf(stderr, "Error loading image '%s'\n", filename);
         exit(1);
    }

    fclose(fp);

    //read pixel data from filename into img. The pixel data is stored in scanline order from left to right (up to bottom) in 3-byte chunks (r g b values for each pixel) encoded as binary numbers.

	return img;
}

/* Create threads and apply filter to image.
 Each thread shall do an equal share of the work, i.e. work=height/number of threads.
 Compute the elapsed time and store it in *elapsedTime (Read about gettimeofday).
 Return: result (filtered image)
 */
PPMPixel *apply_filters(PPMPixel *image, unsigned long w, unsigned long h, double *elapsedTime) {

    PPMPixel *result;
    //allocate memory for result

    //allocate memory for parameters (one for each thread)

    /*create threads and apply filter.
     For each thread, compute where to start its work.  Determine the size of the work. If the size is not even, the last thread shall take the rest of the work.
     */
   

   //Let threads wait till they all finish their work.


	return result;
}


/*The driver of the program. Check for the correct number of arguments. If wrong print the message: "Usage ./a.out filename"
    Read the image that is passed as an argument at runtime. Apply the filter. Print elapsed time in .3 precision (e.g. 0.006 s). Save the result image in a file called laplacian.ppm. Free allocated memory.
 */
int main(int argc, char *argv[])
{
    //printf("%d", argc);
    if(argc != 2){
        printf("Incorrect number of arguments, only 1 image path needed\n");
        exit(1);
    }

    char* file_path = argv[1];

	//load the image into the buffer
    unsigned long int w, h;
    double elapsedTime = 0.0;

    PPMImage *image;
    image = readImage(file_path, &w, &h);

	
	return 0;
}
