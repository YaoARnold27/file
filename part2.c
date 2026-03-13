#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define WIDTH 320
#define HEIGHT 240


int pixel_buffer_start; // global variable
volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
void draw_line(int x, int y, int xx, int yy, short int line_color);
void pixel(int x, int y, short int line_color);
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
void wait_for_sync()
{
    *pixel_ctrl_ptr = 1;
    int status = *(pixel_ctrl_ptr + 3);
    // Polling loop
    while ((status & 0x1) != 0)
        status = *(pixel_ctrl_ptr + 3);
}
void clear_screen() // LOOP all pixel and color to black
{
    for (int i = 0; i < WIDTH; i++)
        for (int j = 0; j < HEIGHT; j++)
            pixel(i, j, (short int)0x0000);
}
int main(void)
{
   
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    int y=10,vy = 1;

    // add front buffer to back buffer
    *(pixel_ctrl_ptr+1) = *(pixel_ctrl_ptr);
    clear_screen();
    while (1)
    {
        draw_line(10,y,30,y,(short int )0); 
        
        
        if (y+vy<=0 || y+vy >= HEIGHT) vy *=-1; /// check if out of page
            y+=vy;   // update coordinate
        draw_line(10,y,30,y,(short int )0xFFFF);   // draw the line with y changing
        wait_for_sync();
        
    }
}

// code not shown for clear_screen() and draw_line() subroutines


void pixel(int x, int y, short int line_color)
{
    volatile short int *one_pixel_address;

        one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);

        *one_pixel_address = line_color;
}


void draw_line(int x, int y, int xx, int yy, short int line_color)
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

    float slp = (yy - y) * 1.0 / (xx - x);  // find slope

    if (slp > 1 || slp < -1) // if slope more than 1, it is steep line draw according to y
    {
        // Find smaller y
        if (yy < y)
        {   //  exchange coordinate
            swap(&x, &xx);
            swap(&y, &yy);
            slp = (yy - y) * 1.0 / (xx - x);
        }
        // Loop the y and calculate and round x
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