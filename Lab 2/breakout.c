unsigned long long __attribute__((used)) VGAaddress = 0xc8000000; // Memory storing pixels
unsigned long long __attribute__((used)) UARTaddress = 0xFF201000;
unsigned int __attribute__((used)) red = 0x0000F0F0;
unsigned int __attribute__((used)) green = 0x00000F0F;
unsigned int __attribute__((used)) blue = 0x000000FF;
unsigned int __attribute__((used)) white = 0x0000FFFF;
unsigned int __attribute__((used)) black = 0x0;
unsigned int __attribute__((used)) pink = 0x0000F81F;   // Pink
unsigned int __attribute__((used)) purple = 0x0000801F; // Purple

#define NCOLS 10     // <- Supported value range: [1,18]
#define NROWS 14     // <- This variable might change.
#define TILE_SIZE 15 // <- Tile size, might change.

char *won = "You Won";                                         // DON'T TOUCH THIS - keep the string as is
char *lost = "You Lost";                                       // DON'T TOUCH THIS - keep the string as is
unsigned short height = 240;                                   // DON'T TOUCH THIS - keep the value as is
unsigned short width = 320;                                    // DON'T TOUCH THIS - keep the value as is
char font8x8[128][8];                                          // DON'T TOUCH THIS - this is a forward declaration
unsigned char tiles[NROWS][NCOLS] __attribute__((used)) = {0}; // DON'T TOUCH THIS - this is the tile map
int barY = 120;
int ballCenterX = 15;
int ballCenterY = 120;
unsigned int ball_playing = 1;
unsigned int dx = 15;    // single speed variable
unsigned int angle = 90; // initial angle is 90 (straight right)
typedef struct _block
{
    unsigned char destroyed;
    unsigned char deleted;
    unsigned int pos_x;
    unsigned int pos_y;
    unsigned int color;
} Block;

struct _block blocks[NCOLS][NROWS];
typedef enum _gameState
{
    Stopped = 0,
    Running = 1,
    Won = 2,
    Lost = 3,
    Exit = 4,
} GameState;
GameState currentState = Stopped;
// helper functions
void default_values()
{
    barY = 120;
    ballCenterX = 15;
    ballCenterY = 120;
    angle = 90;
    ball_playing = 1;
}

static inline int clamp(int v, int low, int high)
{
    if (v < low)
        return low; // If v is less than the minimum, return the minimum
    else if (v > high)
        return high; // If v is greater than the maximum, return the maximum
    else
        return v; // Otherwise, return v unchanged
}

// directly clamps barY
static inline void clamp_bar(void)
{
    barY = clamp(barY, 0, (int)height - 45);
}
// checks if the UART data is valid ("ready")
static inline int uart_ready(unsigned long long v)
{
    return ((v >> 8) & 0xFF) == 0x80;
}

// returns tha actual char typed , like "w"
static inline unsigned char uart_char_typed(unsigned long long v)
{
    return (unsigned char)(v & 0xFF);
}

// returns remaining uart chars
static inline unsigned char uart_remaining(unsigned long long v)
{
    return (unsigned char)((v >> 16) & 0xFF);
}

// Function declarations
// This function sets a pixel at (x_coord, y_coord) to the given color
void SetPixel(unsigned int x_coord, unsigned int y_coord, unsigned int color);
asm("SetPixel: \n\t"
    "LDR R3, =VGAaddress \n\t"
    "LDR R3, [R3] \n\t"
    "LSL R1, R1, #10 \n\t"
    "LSL R0, R0, #1 \n\t"
    "ADD R1, R0 \n\t"
    "STRH R2, [R3,R1] \n\t"
    "BX LR");

void ClearScreen();
asm("ClearScreen: \n\t"
    "PUSH {LR} \n\t"
    "PUSH {R4, R5} \n\t"
    "LDR R2, =white \n\t"
    "LDR R2, [R2] \n\t"
    "MOV R5, #0 \n\t"
    "ClearScreenOuterLoop: \n\t"
    "MOV R4, #0  \n\t"
    "ClearScreenInnerLoop: \n\t"
    "MOV R0, R4 \n\t"
    "MOV R1, R5 \n\t"
    "BL SetPixel \n\t"
    "ADD R4, R4, #1 \n\t"
    "CMP R4, #320 \n\t"
    "BEQ ClearScreenNextRow \n\t"
    "B ClearScreenInnerLoop \n\t"
    "ClearScreenNextRow: \n\t"
    "ADD R5, R5, #1 \n\t"
    "CMP R5, #240 \n\t"
    "BEQ ClearScreenDone \n\t"
    "B ClearScreenOuterLoop \n\t"
    "ClearScreenDone: \n\t"
    "POP {R4, R5} \n\t"
    "POP {LR} \n\t"
    "BX LR");

int ReadUart();
asm("ReadUart:\n\t"
    "LDR R1, =UARTaddress \n\t"
    "LDR R1, [R1] \n\t"
    "LDR R0, [R1] \n\t"
    "BX LR");

void WriteUart(char c);
asm("WriteUart:\n\t"
    "LDR R1, =UARTaddress \n\t"
    "LDR R1, [R1] \n\t"
    "STRH R0, [R1] \n\t"
    "BX LR");

/*
* Draw a block at the specified position with the given dimensions and color
*/
void draw_block(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color)
{
    unsigned int x_end = x + width;
    unsigned int y_end = y + height;

    for (unsigned int yy = y; yy < y_end; yy++)
    {
        for (unsigned int xx = x; xx < x_end; xx++)
        {
            SetPixel(xx, yy, color);
        }
    }
}
/*
 * Draw the blocks at the starting position
 */
void draw_blocks_starting()
{
    for (int i = 0; i < NCOLS; i++)
    {
        for (int j = 0; j < NROWS; j++)
        {
            // blocks[i][j].pos_x = width - (NCOLS * TILE_SIZE) + i * TILE_SIZE;
            blocks[i][j].pos_x = width - i * TILE_SIZE;
            blocks[i][j].pos_y = j * TILE_SIZE;
            blocks[i][j].color = (i + j) % 2 == 0 ? pink : purple;
            blocks[i][j].destroyed = 0;
            blocks[i][j].deleted = 0;

            draw_block(blocks[i][j].pos_x, blocks[i][j].pos_y, 15, 15, blocks[i][j].color);
        }
    }
}
/*
 * Draw the bar at the specified position, reusing draw_block
 */
void draw_bar(unsigned int y)
{

    draw_block(0, y, 7, 45, black);
}
/*
 * Draw the ball at the current position
 */
void draw_ball()
{
    int cx = ballCenterX;
    int cy = ballCenterY;

    // Loop vertically from -3 to +3
    for (int dy = -3; dy <= 3; ++dy)
    {
        // How wide this row is:
        int span = 3 - abs(dy); // 3,2,1,0,1,2,3 → widths 0..3
        for (int dx = -span; dx <= span; ++dx)
        {
            int x = cx + dx;
            int y = cy + dy;
            if (x >= 0 && x < width && y >= 0 && y < height)
                SetPixel(x, y, black);
        }
    }
}
/*
 * Draw the playing field
 */
void draw_playing_field()
{

    for (int column = 0; column < NCOLS; ++column)
    {
        for (int row = 0; row < NROWS; ++row)
        {
            if (blocks[column][row].destroyed == 1)
                continue;
            unsigned int color = ((row + column) & 1) ? pink : purple;
            draw_block((unsigned)blocks[column][row].pos_x, (unsigned)blocks[column][row].pos_y, TILE_SIZE, TILE_SIZE, color);
        }
    }
}


/*
 * Check for collision with the bar
 */
void bar_collide()
{
    if (ballCenterX > 7 || ballCenterY < barY || ballCenterY > barY + 45)
        return; // ball not near bar
    ball_playing = 1;
    // Determine which region of the bar was hit
    int region = (ballCenterY - barY) / 15;

    // move the ball based on region, and if it is retreating or playing
    // Upper third: 45° angle (ball goes up and right)
    if (region == 0)
    {
        angle = 45;
    }
    // Middle third: 90° angle (ball goes straight right)
    else if (region == 1)
    {
        angle = 90;
    }
    // Lower third: 135° angle (ball goes down and right)
    else
    {
        angle = 135;
    }
}

/*
 * Check for collision with the blocks
 */
void block_collide()
{
    for (int i = 0; i < NCOLS; i++)
    {
        for (int j = 0; j < NROWS; j++)
        {
            // already destroyed
            if (blocks[i][j].destroyed == 1)
                continue;

            unsigned int x_pos = blocks[i][j].pos_x;
            unsigned int y_pos = blocks[i][j].pos_y;

            // Check collision with ball's 3-pixel radius from all sides

            int x_ball_center = ballCenterX + 3;
            int y_ball_center = ballCenterY + 3;

            // Check if ball overlaps with block (15x15 pixels)
            if (x_ball_center < x_pos || x_ball_center > x_pos + 15 || y_ball_center > y_pos + 15 || y_ball_center < y_pos)
                continue; // No collision

            // Collision detected - mark block destroyed
            blocks[i][j].destroyed = 1;

            // ball switches between playing and retreating
            ball_playing = !ball_playing;
            // return;
        }
    }
}

/*
 * Check for wall bounce (top/bottom)
 */
void wall_bounce_check()
{

    if (ballCenterY >= height - 7 || ballCenterY <= 0)
    {
        angle = 180 - angle;
        if (ballCenterY >= height - 7)
            ballCenterY = height - 7;
        else
            ballCenterY = 0;
    }
}

/*
 * Update the game state
*/
void update_game_state()
{
    if (currentState != Running)
        return;

    // move the ball
    int step = 15;

    if (ball_playing == 1) // moving right
    {
        if (angle == 45)
        {
            ballCenterX += step;
            ballCenterY -= step;
        }
        else if (angle == 90)
        {
            ballCenterX += step;
        }
        else if (angle == 135)
        {
            ballCenterX += step;
            ballCenterY += step;
        }
    }
    else if (ball_playing == 0) // moving left
    {
        if (angle == 45)
        {
            ballCenterX -= step;
            ballCenterY -= step;
        }
        else if (angle == 90)
        {
            ballCenterX -= step;
        }
        else if (angle == 135)
        {
            ballCenterX -= step;
            ballCenterY += step;
        }
    }

    //  check top/bottom wall bounce 
    wall_bounce_check();
    

    if (ball_playing == 1)
    {
        block_collide();
    } // if going right, check blocks
    else
    {
        bar_collide();
    } // if going left, check bar

    // check WIN/LOSE
    if (ballCenterX + 3 >= (int)width)
    {
        currentState = Won;
        return;
    }

    // 7 + 3 (radius)
    if (ballCenterX < 7 + 3 && (ballCenterY < barY - 3 || ballCenterY > barY + 45 + 3))
    {
        currentState = Lost;
        return;
    }
}

// move the bar
void update_bar_state()
{
    // Read until no more ready data this tick
    for (;;)
    {
        unsigned long long v = ReadUart();
        if (!uart_ready(v))
            return;

        unsigned char ch = uart_char_typed(v);
        unsigned char rem = uart_remaining(v);

        for (int i = -1; i < (int)rem; ++i)
        {
            if (ch == 'w')
            {
                barY -= TILE_SIZE;
                clamp_bar();
                if (currentState == Stopped)
                    currentState = Running;
            }
            else if (ch == 's')
            {
                barY += TILE_SIZE;
                clamp_bar();
                if (currentState == Stopped)
                    currentState = Running;
            }
            else if (ch == '\r' || ch == '\n')
            {
                currentState = Exit;
                return;
            }

            if (i + 1 < (int)rem)
            {
                v = ReadUart();
                ch = uart_char_typed(v);
            }
        }
    }
}

void write(char *str)
{
    if (!str)
        return;
    while (*str)
        WriteUart(*str++);
    WriteUart('\n');
}

void play()
{
    draw_blocks_starting();
    while (1)
    {
        ClearScreen();
        update_game_state();
        update_bar_state();
        if (currentState != Running)
        {
            break;
        }

        draw_playing_field();
        // draw bar and ball
        draw_ball();
        draw_bar((unsigned)barY);
    }
    if (currentState == Won)
    {
        write(won);
    }
    else if (currentState == Lost)
    {
        write(lost);
    }
    else if (currentState == Exit)
    {
        return;
    }
    currentState = Stopped;
}

// It must initialize the game
void reset()
{
    int remaining = 0;
    do
    {
        unsigned long long out = ReadUart();

        // If UART not ready, nothing to drain
        if (!uart_ready(out))
            return;

        remaining = uart_remaining(out);
    } while (remaining > 0);

    // Validate NCOLS
    if (NCOLS < 1 || NCOLS > 18)
    {
        write("Invalid NCOLS (1..18 required).");
        currentState = Exit;
        return;
    }

    default_values();
}

void wait_for_start()
{
    currentState = Stopped;
    while (currentState != Running)
    {
        unsigned long long out = ReadUart();
        if (!uart_ready(out))
            continue;
        // character written
        unsigned char ch = uart_char_typed(out);
        if ((ch == 'w' || ch == 's'))
        {
            currentState = Running;
        }
        else if (ch == '\r' || ch == '\n')
        {
            currentState = Exit;
            return;
        }
    }
}

int main(void)
{
    ClearScreen();

    while (1)
    {
        wait_for_start();
        default_values();
        play();
        reset();

        if (currentState == Exit)
            break;
    }
    return 0;
}