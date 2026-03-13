#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define WIDTH 320
#define HEIGHT 240


int pixel_buffer_start; // global variable

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

void clear_screen() // LOOP all pixel and color to black
{
    for (int i = 0; i < WIDTH; i++)
        for (int j = 0; j < HEIGHT; j++)
            pixel(i, j, (short int)0x0000);
}
int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    clear_screen();
    draw_line(0, 0, 150, 150, 0x001F);   // this line is blue
    draw_line(150, 150, 319, 0, 0x07E0); // this line is green
    draw_line(0, 239, 319, 239, 0xF800); // this line is red
    draw_line(319, 0, 0, 239, 0xF81F);   // this line is a pink color
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