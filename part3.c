// Set predefined constants
#define WIDTH 320
#define HEIGHT 240
#define NUM_BOX 10
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#include <stdlib.h>
#include <math.h>

int my_round(float x)
{
    if (x >= 0)
        return (int)(x + 0.5f);
    else
        return (int)(x - 0.5f);
}
void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}
// Variables
int x[NUM_BOX];
int y[NUM_BOX];
int dx[NUM_BOX];
int dy[NUM_BOX];
short int color[NUM_BOX];
volatile int BACK;
volatile int *PIXEL_CTR_PTR = (int *)0xFF203020;


short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];




short int rainbow[7] = {
    0xF800, // Red
    0xFC00, // Orange
    0xFFE0, // Yellow
    0x07E0, // Green
    0x001F, // Blue
    0x481F, // Indigo
    0x801F  // Violet
};
/*
Obtain the address of pixel and assign the color
*/
void pixel(int x, int y, short int line_color)
{
    // if out of page , disgard this point
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;

    
    volatile short int *pixel_address;
    pixel_address = (volatile short int *) (BACK + (y << 10) + (x << 1)); // Get the memory of pixel
    *pixel_address = line_color;                               // Put color into memory
    return;
}

// Clear the screen by write Black everywhere
void clear_screen()
{
    for (int i = 0; i < WIDTH; i++)
        for (int j = 0; j < HEIGHT; j++)
            pixel(i, j, (short int)0x0000);
}
// Check the board and reflect the speed if out of page
void checkboard(int i)
{
    if ((x[i] + dx[i]) <= 0)
        dx[i] *= -1;
    if ((x[i] + dx[i] + 1) >= WIDTH)
        dx[i] *= -1;
    if ((y[i] + dy[i]) <= 0)
        dy[i] *= -1;
    if ((y[i] + dy[i] + 1) >= HEIGHT)
        dy[i] *= -1;
}

void line(int x, int y, int xx, int yy, short int line_color)
{
    // Vertical line avoid division 0
    if (x == xx)
    {
        for (int i = MIN(y, yy); i <= MAX(y, yy); ++i)
            pixel(x, i, line_color);
        return;
    }
    // Horizontal line
    if (y == yy)
    {
        for (int i = MIN(x, xx); i <= MAX(x, xx); ++i)
            pixel(i, y, line_color);
        return;
    }

    float slp = (yy - y) * 1.0 / (xx - x);

    if (slp > 1 || slp < -1) // if slope more than 1, it is steep line draw according to y
    {
        // Find smaller y
        if (yy < y)
        {
            swap(&x, &xx);
            swap(&y, &yy);
            slp = (yy - y) * 1.0 / (xx - x);
        }

        for (int j = y; j <= yy; j++)
        {
            float i = x + (j - y) / slp;
            i = my_round(i);
            pixel(i, j, line_color);
        }
    }
    else
    {

        // Find smaller x
        if (xx < x)
        {
            swap(&x, &xx);
            swap(&y, &yy);
            slp = (yy - y) * 1.0 / (xx - x);
        }

        for (int i = x; i <= xx; ++i)
        {
            float j = y + (i - x) * slp;
            j = my_round(j);
            pixel(i, j, line_color);
        }
    }
}

void wait_for_sync()
{
    *PIXEL_CTR_PTR = 1;
    int status = *(PIXEL_CTR_PTR + 3);
    // Polling loop
    while ((status & 0x1) != 0)
        status = *(PIXEL_CTR_PTR + 3);
}
void draw()
{
    clear_screen();
    for (int i = 0; i < NUM_BOX; ++i)
    {
        // draw the box
        pixel(x[i], y[i], color[i]);
        pixel(x[i] + 1, y[i], color[i]);
        pixel(x[i] + 1, y[i] + 1, color[i]);
        pixel(x[i], y[i] + 1, color[i]);
        // draw the line
        line(x[i], y[i], x[(i + 1) % NUM_BOX], y[(i + 1) % NUM_BOX], (short int)0xFFFF);

        checkboard(i);
        x[i] += dx[i];
        y[i] += dy[i];
    }
}

int main()
{

    // Set random color
    
    for (int i = 0; i < NUM_BOX; i++)
    {
        // set random direction

        dx[i] = (rand() % 2) * 2 - 1;
        dy[i] = (rand() % 2) * 2 - 1;
        color[i] = rainbow[rand() % 7];
        // randome color and pos
        x[i] = rand() % (WIDTH - 1);
        y[i] = rand() % (HEIGHT - 1);
    }
    
    *(PIXEL_CTR_PTR + 1) = (int) &Buffer1;  // store the address to back buffer
    wait_for_sync();    //exchange
    BACK = *PIXEL_CTR_PTR;
    clear_screen();

    *(PIXEL_CTR_PTR+1) = (int)&Buffer2; // store buffer 2 in back buffer, front is buffer1
    BACK = *PIXEL_CTR_PTR;
    clear_screen();
    while (1)
    {

        draw(); //draw on back
        wait_for_sync();
        BACK = *(PIXEL_CTR_PTR + 1);
    }

    return EXIT_SUCCESS;
}