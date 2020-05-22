#include "helpers.h"
#include <stdio.h>

// Declare prototype for rounding floats to nearest number
int my_round(float number);
// Declare proptype for capping a number
int cap_num(int number, int maximum);

RGBTRIPLE help_blur(RGBTRIPLE current_pixel, int current_height, int current_width,  int image_height, int image_width, 
                    RGBTRIPLE orig_img_array[image_height][image_width]);
// Declare prototype which calculates limits for blur filter
int calc_limit(int lower_bound, int upper_bound, int current_index);

// Declare prototype to calculate mapping of mini pixel for blur filter
int get_map_coord(int current_height_or_width, int h_or_w, int height_or_width_of_array, int height_or_width_of_image);

// The third argument is a 2d array with the name of 'image' and type RGBTRIPLE
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            RGBTRIPLE yoo = image[h][w];
            int blu = yoo.rgbtBlue;
            int gre = yoo.rgbtGreen;
            int red = yoo.rgbtRed;

            // Make total into a float
            float total = (blu + gre + red);
            float avg_float = total / 3;
            // Round the float
            int avg = my_round(avg_float);

            // printf("iteration Old %i: B:%i, G:%i, B:%i, AvFlo: %f, Av: %i\n", i, blu, gre, red, avg_float, avg);
            // Try mutating the original array
            image[h][w].rgbtBlue = avg;
            image[h][w].rgbtGreen = avg;
            image[h][w].rgbtRed = avg;
        }
    }

    // Function doesn't return anything as it modifies array inside the function
    return;
    // Return modified 2d array
}

// Convert image to sepia
void sepia(int height, int width, RGBTRIPLE image[height][width])
{
    for (int w = 0; w < width; w++)
    {
        for (int h = 0; h < height; h++)
        {
            RGBTRIPLE original = image[h][w];
            int orig_blu = original.rgbtBlue;
            int orig_gre = original.rgbtGreen;
            int orig_red = original.rgbtRed;

            // Also include capping at 255
            // sepiaBlue = .272 * originalRed + .534 * originalGreen + .131 * originalBlue
            float flo_blu = .272 * orig_red + .534 * orig_gre + .131 * orig_blu;
            float flo_gre = .349 * orig_red + .686 * orig_gre + .168 * orig_blu;
            float flo_red = .393 * orig_red + .769 * orig_gre + .189 * orig_blu;

            int sepia_blu = cap_num(my_round(flo_blu), 255);
            int sepia_gre = cap_num(my_round(flo_gre), 255);
            int sepia_red = cap_num(my_round(flo_red), 255);

            // printf("iteration Old %i: B:%i, G:%i, B:%i, AvFlo: %f, Av: %i\n", i, blu, gre, red, avg_float, avg);
            // Mutate original array
            image[h][w].rgbtBlue = sepia_blu;
            image[h][w].rgbtGreen = sepia_gre;
            image[h][w].rgbtRed = sepia_red;
        }
    }

    return;
}

void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    // Have a temp array in which you can start creating a new image
    RGBTRIPLE tmp[height][width];

    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            // Capture the original properties
            RGBTRIPLE original = image[h][w];
            int orig_blu = original.rgbtBlue;
            int orig_gre = original.rgbtGreen;
            int orig_red = original.rgbtRed;

            // Create new horizontal location for these attributes. Think about zero indexing. You need to minus 1
            int new_hor = width - w - 1;
            // printf("height: %i, new hor: %i, width of photo: %i, current w: %i\n", h, new_hor, width, w);

            tmp[h][new_hor].rgbtBlue = orig_blu;
            tmp[h][new_hor].rgbtGreen = orig_gre;
            tmp[h][new_hor].rgbtRed = orig_red;
        }
    }

    // Loop 2: Now that the tmp array is full, repopulate the original array
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            image[h][w].rgbtBlue = tmp[h][w].rgbtBlue;
            image[h][w].rgbtGreen = tmp[h][w].rgbtGreen;
            image[h][w].rgbtRed = tmp[h][w].rgbtRed;
        }
    }
    
    return;
}

// Blur image
/*
There are a number of ways to create the effect of blurring or softening an image. For this problem, we’ll use the “box blur,” which works by taking each pixel and, for each color value, giving it a new value by averaging the color values of neighboring pixels.

The new value of each pixel would be the average of the values of all of the pixels that are within 1 row and column of the original pixel (forming a 3x3 box). For example, each of the color values for pixel 6 would be obtained by averaging the original color values of pixels 1, 2, 3, 5, 6, 7, 9, 10, and 11 (note that pixel 6 itself is included in the average). Likewise, the color values for pixel 11 would be be obtained by averaging the color values of pixels 6, 7, 8, 10, 11, 12, 14, 15 and 16.

For a pixel along the edge or corner, like pixel 15, we would still look for all pixels within 1 row and column: in this case, pixels 10, 11, 12, 14, 15, and 16.
*/

/*
An example: [50, 50], we should take the average of the pixels at 
[51, 49] [51, 50] [51, 51]
[50, 49] [50, 50] [50, 51]
[49, 49] [49, 50] [49, 51]
*/

void blur(int height, int width, RGBTRIPLE image[height][width])
{
    // Have a temp array in which you can start creating a new image
    RGBTRIPLE tmp[height][width];

    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            RGBTRIPLE current_pixel = image[h][w];

            // Use helper function and pass in the current height and width of the pixel that is being analysed as well as the height and width of the whole picture
            RGBTRIPLE returned_triple = help_blur(current_pixel, h, w, height, width, image);

            int avg_blu = returned_triple.rgbtBlue;
            int avg_gre = returned_triple.rgbtGreen;
            int avg_red = returned_triple.rgbtRed;

            tmp[h][w].rgbtBlue = avg_blu;
            tmp[h][w].rgbtGreen = avg_gre;
            tmp[h][w].rgbtRed = avg_red;
        }
    }

    // Loop 2: Now that the tmp array is full, repopulate the original array
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            image[h][w].rgbtBlue = tmp[h][w].rgbtBlue;
            image[h][w].rgbtGreen = tmp[h][w].rgbtGreen;
            image[h][w].rgbtRed = tmp[h][w].rgbtRed;
        }
    }

    return;
}

int my_round(float number)
{
    return (number >= 0) ? (int)(number + 0.5) : (int)(number - 0.5);
}

int cap_num(int number, int maximum)
{
    if (number < maximum)
    {
        return number;
    } 
    else 
    {
        return maximum;
    }
}

RGBTRIPLE help_blur(RGBTRIPLE current_pixel, int current_height, int current_width, int image_height, int image_width, 
                    RGBTRIPLE orig_img_array[image_height][image_width])
{
    // Calculate the number of pixels that need to be taken into consideration when calculating the average in order for the blur effect to work
    int height_of_array = calc_limit(0, image_height - 1, current_height);
    int width_of_array  = calc_limit(0, image_width - 1, current_width);

    // Declare new mini array of which will hold the values of the pixels that surround the current pixel
    RGBTRIPLE surrounding_array[height_of_array][width_of_array];

    // printf("surrounding array: h: %i, w: %i\n", height_of_array, width_of_array);

    // Declare ints that will hold the totals of each amount of the colors of the surrounding pixels
    float total_blu = 0, total_gre = 0, total_red = 0;

    // Loop through the new empty array and allocate the values of the new pixels to the values of the surrounding pixels from the original array
    for (int h = 0; h < height_of_array; h++)
    {
        for (int w = 0; w < width_of_array; w++)
        {

            // int height_mapped = current_height + h - 1; // <= works for middle pixels
            // int width_mapped = current_width + w - 1; // <= works for middle pixels

            int height_mapped = get_map_coord(current_height, h, height_of_array, image_height);
            int width_mapped = get_map_coord(current_width, w, width_of_array, image_width);

            // Extract the properties from the original array
            RGBTRIPLE mapped_pixel = orig_img_array[height_mapped][width_mapped];
            int mapped_blu = mapped_pixel.rgbtBlue;
            int mapped_gre = mapped_pixel.rgbtGreen;
            int mapped_red = mapped_pixel.rgbtRed;

            // Assign the mini array the values from the mapped array
            surrounding_array[h][w].rgbtBlue = mapped_blu;
            surrounding_array[h][w].rgbtGreen = mapped_gre;
            surrounding_array[h][w].rgbtRed = mapped_red;
        }
    }

    /* --- For Debugging Purposes Only --- */
    // You now have a mini array that has been allocated values from the original array through a mapping process. You should now loop through the small array and add the colors to the totals. Loop through the new surrounding_array and print out the values
    for (int h = 0; h < height_of_array; h++)
    {
        for (int w = 0; w < width_of_array; w++)
        {
            RGBTRIPLE moo = surrounding_array[h][w];
            // printf("surrounding_array: %i %i %i %i\n", h, w, moo.rgbtBlue, moo.rgbtGreen);
            int moo_blu = moo.rgbtBlue;
            int moo_gre = moo.rgbtGreen;
            int moo_red = moo.rgbtRed;
            // printf("surr h=%i w=%i b=%i g=%i r=%i\n", h, w, moo_blu, moo_gre, moo_red);
        }
    }
    /* --- End of Debugging */

    // Go through the mini array (surrounding_array) and add up the totals of the values of red, green and red
    for (int h = 0; h < height_of_array; h++)
    {
        for (int w = 0; w < width_of_array; w++)
        {
            total_blu += surrounding_array[h][w].rgbtBlue;
            total_gre += surrounding_array[h][w].rgbtGreen;
            total_red += surrounding_array[h][w].rgbtRed;
        }
    }

    int count = height_of_array * width_of_array;
    float avg_blu_flo = total_blu / count; 
    float avg_gre_flo = total_gre / count;
    float avg_red_flo = total_red / count;
    
    int avg_blu_int = my_round(avg_blu_flo);
    int avg_gre_int = my_round(avg_gre_flo);
    int avg_red_int = my_round(avg_red_flo);
    
    RGBTRIPLE new_pixel;
    new_pixel.rgbtBlue = avg_blu_int;
    new_pixel.rgbtGreen = avg_gre_int;
    new_pixel.rgbtRed = avg_red_int;
    // printf("new pixel is b:%i, g:%i, r:%i\n", new_pixel.rgbtBlue, new_pixel.rgbtGreen, new_pixel.rgbtRed);

    // printf("Returning the new pixel r:%i g:%i b:%i\n", new_pixel.rgbtBlue, new_pixel.rgbtGreen, new_pixel.rgbtRed);

    return new_pixel;
}

int calc_limit(int lower_bound, int upper_bound, int current_index)
{
    // There is only one row or column.
    if (lower_bound == upper_bound)
    {
        return 1;
    }
    // Pixel is located on top or bottom row or column. Minus one because of zero indexing
    else if (current_index == lower_bound || current_index == upper_bound)
    {
        return 2;
    }
    // Any other scenario
    else
    {
        return 3;
    }
}

int get_map_coord(int current_height_or_width, int h_or_w, int height_or_width_of_array, int height_or_width_of_image)
{
    int return_value;

    // This works if the pixel is in the middle (i.e. the mini array is 3 x 3)
    // You are effectivity telling the 3x3 mini array to start (-1, -1) below the (0,0 point)
    if (height_or_width_of_array == 3)
    {
        // return_value = current_height_or_width + h_or_w - 1;

        if (current_height_or_width == height_or_width_of_array)
        {
            return_value = current_height_or_width + h_or_w;
        }
        else
        {
            return_value = current_height_or_width + h_or_w - 1;
        }
    }
    // Currently things work when the pixel is 0,0
    // Things also work when the pixel is 0,1
    else if (height_or_width_of_array == 2)
    {
        // If the position right now is at the bottom of the picture or to the right of the image, the mini array should start one back. Remember, to check if the current position is at the edge you need to compare it with the width of height minus one
        if (current_height_or_width == height_or_width_of_image - 1)
        {
            return_value = current_height_or_width + h_or_w - 1;
        }
        else
        {
            return_value = current_height_or_width + h_or_w;
        }
    }
    else
    {
        return_value = current_height_or_width + h_or_w;
    }

    return return_value;

}