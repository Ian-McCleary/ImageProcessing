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

/*  Tom Mclaughlin
    Ian McCleary
*/

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
void *threadfn(void *para){
    //cast struct to void *
    struct parameter *params;
    params = (struct parameter*)para;

    int laplacian[FILTER_WIDTH][FILTER_HEIGHT] =
	{
	  -1, -1, -1,
	  -1,  8, -1,
	  -1, -1, -1,
	};
   
    int imageWidth = (*params).w;
    int imageHeight = (*params).h;
    int red, green, blue;
    int iteratorFilterWidth, iteratorFilterHeight, iteratorImageWidth, iteratorImageHeight;
    int x_coordinate;
    int y_coordinate;

    
    for(iteratorImageHeight = (*params).start; iteratorImageHeight < ((*params).start + (*params).size); iteratorImageHeight++){ //y of image starting at point

      for(iteratorImageWidth = 0; iteratorImageWidth < imageWidth; iteratorImageWidth++){//x of image 

         for(iteratorFilterHeight = 0; iteratorFilterHeight < FILTER_HEIGHT; iteratorFilterHeight++){ //y of filter

            y_coordinate = (iteratorImageHeight - FILTER_HEIGHT / 2 + iteratorFilterHeight + imageHeight) % imageHeight;

            for(iteratorFilterWidth = 0; iteratorFilterWidth < FILTER_WIDTH; iteratorFilterWidth++){//x of filter
                
               x_coordinate = (iteratorImageWidth - FILTER_WIDTH / 2 + iteratorFilterWidth + imageWidth) % imageWidth;
               red+= (*params).image[y_coordinate * imageWidth + x_coordinate].r * laplacian[iteratorFilterHeight][iteratorFilterWidth];
               green+= (*params).image[y_coordinate * imageWidth + x_coordinate].g * laplacian[iteratorFilterHeight][iteratorFilterWidth];
               blue+= (*params).image[y_coordinate * imageWidth + x_coordinate].b * laplacian[iteratorFilterHeight][iteratorFilterWidth];     
      }
    }
         //confirming rgb is within min/max
         if(red < 0){
            red = 0; 
         }else if(red > 255){
            red = 255;
         }
         if(green < 0) {
            green = 0;
         }else if(green > 255){
            green = 255;
         }
         if(blue < 0){
            blue = 0;
         }else if(blue > 255) {
            blue = 255;
         }
         //storing result
         if (iteratorImageHeight * imageWidth + iteratorImageWidth > imageHeight * imageWidth){
             break;
         } else{
            (*params).result[iteratorImageHeight * imageWidth + iteratorImageWidth].r = red;
            (*params).result[iteratorImageHeight * imageWidth + iteratorImageWidth].g = green;
            (*params).result[iteratorImageHeight * imageWidth + iteratorImageWidth].b = blue;
         }
         red = 0;
         green = 0;
         blue = 0;
      }
    }
	pthread_exit(NULL);
}

/*Create a new P6 file to save the filtered image in. Write the header block
 e.g. P6
      Width Height
      Max color value
 then write the image data.
 The name of the new file shall be "name" (the second argument).
 */

void writeImage(PPMPixel *image, char *name, unsigned long int width, unsigned long int height){

    FILE* fp = fopen(name, "w+");
    fprintf(fp, "P6\n%lu %lu\n255\n", width, height);
    fwrite(image, 1, sizeof(PPMPixel)* width * height, fp);
    fclose(fp);
    return;
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

PPMPixel *readImage(const char *filename, unsigned long int **width, unsigned long int **height){

	PPMImage *img;
    //PPMPixel *img2;

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
    //allocate memory for img. NOTE: A ppm image of w=200 and h=300 will contain 60000 triplets (i.e. for r,g,b), ---> 18000 bytes.

    //allocate memory for entire image based on ppmimage struct
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img){
        printf("Memory allocation failed\n");
        exit(1);
    }
    //If there are comments in the file, skip them. You may assume that comments exist only in the header block.
   //skip comments (only skips 1 commnet, could be an issue)
    c = getc(fp);
    if (c == '#'){
        fscanf(fp, "%*[^\n]");
    }
    //read image size information
    
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
        printf("Image size: %d, %d\n", img->x, img->y);
        fprintf(stderr, "Invalid image size\n");
        exit(1);
    }

    *width = malloc(sizeof(unsigned long int));
    *height = malloc(sizeof(unsigned long int));

    **width = (unsigned long int)img->x;
    **height = (unsigned long int)img->y;

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
         fprintf(stderr, "Invalid rgb component\n");
         exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color!= 255) {
         fprintf(stderr, "Does not have 8-bits components\n");
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

	return img->data;
}

/* Create threads and apply filter to image.
 Each thread shall do an equal share of the work, i.e. work=height/number of threads.
 Compute the elapsed time and store it in *elapsedTime (Read about gettimeofday).
 Return: result (filtered image)
 */
PPMPixel *apply_filters(PPMPixel *image, unsigned long w, unsigned long h, double **elapsedTime) {

    PPMPixel *result;
    double num_rows = w / THREADS;
    int thread_rows = num_rows;

    pthread_t *id[THREADS];

    struct parameter *params;
    //allocate memory for parameters (one for each thread). Need to free space afterwards.
    params = malloc(sizeof(struct parameter)*THREADS);
    //allocate memory for result
    result = malloc(sizeof(PPMPixel) * w * h);

    *elapsedTime = malloc(sizeof(double));
    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);

    //divide work
    int rc;
    for(int i = 0; i < THREADS; i++){
        
        params[i].start = i *thread_rows;
        params[i].size = thread_rows;
        params[i].image = image;
        params[i].result = result;
        params[i].w = w;
        params[i].h = h;
        /*create threads and apply filter.
        For each thread, compute where to start its work.  Determine the size of the work. If the size is not even, the last thread shall take the rest of the work.
        */
        rc = pthread_create((pthread_t *)&id[i], NULL, threadfn, &params[i]);
        if (rc){
            printf("Error unable to create thread, %d\n", rc);
            exit(-1);
        }
    }
    //Let threads wait till they all finish their work.
    for(int x = 0; x < THREADS; x++){
        pthread_join((pthread_t)id[x], NULL);
    }
    //end time caulculation
    gettimeofday(&endTime, NULL);
    double tt = (double)(endTime.tv_usec - startTime.tv_usec);
    double t1 = trunc(tt/1000);
    double bro = t1/1000;
    **elapsedTime = bro;
    //free parameters
    free(params);
	return result;
}

/*The driver of the program. Check for the correct number of arguments. If wrong print the message: "Usage ./a.out filename"
    Read the image that is passed as an argument at runtime. Apply the filter. Print elapsed time in .3 precision (e.g. 0.006 s). Save the result image in a file called laplacian.ppm. Free allocated memory.
 */
int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Incorrect number of arguments, only 1 image path needed\n");
        exit(1);
    }

    //load the image into the buffer
    char* file_path = argv[1];
    char* outfile = "laplacian.ppm";

    unsigned long int* width;
    unsigned long int* height;

    double *elapsedTime;

    PPMPixel *image;
    PPMPixel *result;
    image = readImage(file_path, &width, &height);

    result = apply_filters(image, *width, *height, &elapsedTime);
    printf("Total time: %g seconds\n", *elapsedTime);
    writeImage(result, outfile, *width, *height);
    //free memory
    free(image);
    free(result);
	
	return 0;
}
