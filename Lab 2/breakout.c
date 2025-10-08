#include <stdlib.h>

// these variable are attribute , colo
unsigned long long __attribute__((used)) VGAaddress = 0xc8000000; // Memory storing pixels
unsigned int __attribute__((used)) red = 0x0000F0F0;
unsigned int __attribute__((used)) green = 0x00000F0F;
unsigned int __attribute__((used)) blue = 0x000000FF;
unsigned int __attribute__((used)) white = 0x0000FFFF;
unsigned int __attribute__((used)) black = 0x0;
unsigned int  __attribute__((used)) pink = 0x0000F81F;   // Pink
unsigned int  __attribute__((used)) purple = 0x0000801F; // Purple

#define NCOLS 10 // <- Supported value range: [1,18]
#define NROWS 14 // <- This variable might change.
#define TILE_SIZE 15 // <- Tile size, might change.

char* won = "You Won";       // DON'T TOUCH THIS - keep the string as is
char* lost = "You Lost";     // DON'T TOUCH THIS - keep the string as is
unsigned short height = 240; // DON'T TOUCH THIS - keep the value as is
unsigned short width = 320;  // DON'T TOUCH THIS - keep the value as is
char font8x8[128][8];        // DON'T TOUCH THIS - this is a forward declaration
unsigned char tiles[NROWS][NCOLS] __attribute__((used)) = { 0 }; // DON'T TOUCH THIS - this is the tile map
/**************************************************************************************************/

/***
 * TODO: Define your variables below this comment
 */
int barY = 120;
int ballCenterX = 15;
int ballCenterY = 120;
int dx = 1, dy = -1; // ball velocity




typedef struct _block
{
    unsigned char destroyed;
    unsigned char deleted;
    unsigned int pos_x;
    unsigned int pos_y;
    unsigned int color;
} Block;

struct _block blocks[NCOLS][NROWS]; // 10 columns to fill 150px width
typedef enum _gameState
{
    Stopped = 0,
    Running = 1,
    Won = 2,
    Lost = 3,
    Exit = 4,
} GameState;
GameState currentState = Stopped;
//helper functions
// Ensures that v stays between low and high
void set_default_values()
{
    barY = 120;
    ballCenterX = 15;
    ballCenterY = 120;
    dx = 1;
    dy = -1;
}
static inline int clamp(int v, int low, int high)
{
    if (v < low)
        return low;    // If v is less than the minimum, return the minimum
    else if (v > high)
        return high;    // If v is greater than the maximum, return the maximum
    else
        return v;     // Otherwise, return v unchanged
}

//directly clamps barY
static inline void clamp_bar(void)
{
	barY = clamp(barY, 0, (int)height - 45);
}
//checks if the UART data is valid ("ready")
static inline int uart_ready(unsigned long long v)
{
	return ((v >> 8) & 0xFF) == 0x80;
}

// returns tha actual char typed , like "w"
static inline unsigned char uart_char_typed(unsigned long long v)
{
	return (unsigned char)(v & 0xFF);
}

//returns remaining uart chars
static inline unsigned char uart_remaining(unsigned long long v)
{ 
	return (unsigned char)((v >> 16) & 0xFF);
}

//returns x coordinate of left edge  leftmost block column
static inline int leftmost_col_x(void){
    return width - NCOLS * TILE_SIZE;
}

//checks if (x,y)i nside a rectangle starting at (x0,y0) with width w and height h.
//needed to see if ball touched 
static inline int in_rectangle(int x, int y, int x0, int y0, int w, int h){
    return (x >= x0 && x < x0 + w && y >= y0 && y < y0 + h);
}

static int corner_hits_block(int ccx, int ccy, int x0)
{
    int pts[4][2] = {{ccx, ccy-3}, {ccx+3, ccy}, {ccx, ccy+3}, {ccx-3, ccy}};
    for (int k = 0; k < 4; ++k) {
        int px = pts[k][0], py = pts[k][1];
        if (px >= x0 && px < 320 && py >= 0 && py < 240) {
            int col = (px - x0) / TILE_SIZE;
            int row = py / TILE_SIZE;
            if (row >= 0 && row < 16 && col >= 0 && col < NCOLS && alive[row][col])
                return 1;
        }
    }
    return 0;
}

/***
 * Here follow the C declarations for our assembly functions
 */

// Function declarations
void draw_block(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color);

	// This function sets a pixel at (x_coord, y_coord) to the given color
void SetPixel(unsigned int x_coord, unsigned int y_coord, unsigned int color);
// assumes R0 = x-coord, R1 = y-coord, R2 = colorvalue
asm("SetPixel: \n\t"
    "LDR R3, =VGAaddress \n\t"
    "LDR R3, [R3] \n\t"
    "LSL R1, R1, #10 \n\t"
    "LSL R0, R0, #1 \n\t"
    "ADD R1, R0 \n\t"
    "STRH R2, [R3,R1] \n\t"
    "BX LR");

void ClearScreen();
 // It must only clear the VGA screen, and not clear any game state
asm("ClearScreen: \n\t"
	// stack preserves the caller's state.
    "    PUSH {LR} \n\t"
    "    PUSH {R4, R5} \n\t"
	//loading where the address of the framebuffer is stored.
	//LDR R4, [R4] dereferences it to get the value stored there, 
	//the actual framebuffer base pointer (0xC8000000).
	" LDR R4, = VGAaddress \n\t"
 	"    LDR  R4, [R4]          \n\t"   // R4 = base
	//var for the color that will be added - FIX: Changed to 0 for black
    "    MOV  R5, #0xFFFF       \n\t"   // color white
    "    MOV  R6, #0            \n\t"   // y = 0
	//
	"1: \n\t" // row loop 
	// row_ptr (R7) = base + y*1024
	"ADD R7, R4, R6, LSL #10 \n\t"
	// x = 0
	" MOV R0, #0 \n\t" 
	
	"2: \n\t" // pixel loop (320 pixels) 
	//store the least significant 16 bits, color(R5) is 16 bits
	// So this writes the black pixel at (x, y).
	"ADD  R1, R7, R0, LSL #1 \n\t"   // R1 = row_ptr + x*2
    "STRH R5, [R1] \n\t"

	// go forward by 1 for x
	" ADD R0, R0, #1 \n\t" 
	// check if x is lesser than 320, as that means end of row
	" CMP R0, #320 \n\t" 
	// branch to 2 if true, b backward
	" BLT 2b \n\t" 
	// here if end of row of x, go to next column
	" ADD R6, R6, #1 \n\t"
	// stop if end of all rows
	" CMP R6, #240 \n\t" 
	//if not go back
	" BLT 1b \n\t"
	 "    MOV  R0, #0x10000 \n\t"
    "    POP {R4,R5}\n\t"
    "    POP {LR} \n\t"
    "    BX LR");
int ReadUart();
asm("ReadUart:\n\t"
    "LDR R1, =0xFF201000 \n\t"
    "LDR R0, [R1]\n\t"
    "BX LR");
void WriteUart(char c);
// R0 = char
asm("WriteUart:              \n\t"
    "    PUSH {LR}           \n\t"
    "1:                      \n\t"
    "    LDR  R1, =0xFF201004 \n\t"   // CONTROL
    "    LDR  R2, [R1]        \n\t"
    "    LSRS R2, R2, #16     \n\t"   // WSPACE
    "    BEQ  1b              \n\t"   // wait while 0
    "    LDR  R1, =0xFF201000 \n\t"   // DATA
    "    STR  R0, [R1]        \n\t"   // write byte (low 8 bits used)
    "    POP  {LR}            \n\t"
    "    BX   LR              \n\t");

	



void init_blocks()
{
    for (int i = 0; i < NCOLS; i++)
    {
        for (int j = 0; j < NROWS; j++)
        {
            blocks[i][j].pos_x = width - i * 15;
            blocks[i][j].pos_y = j * 15;
            blocks[i][j].color = red; // they will have a gradient color
            // blocks[i][j].color = 0x0000F0F0;
            blocks[i][j].destroyed = 0;
            blocks[i][j].deleted = 0;

            draw_block(blocks[i][j].pos_x, blocks[i][j].pos_y, 15, 15, blocks[i][j].color);
        }
    }
}

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
	
void draw_bar(unsigned int y)
{
    // the bar is 7x45 pixels
    draw_block(0, y, 7, 45, blue);
}

  	   
void draw_ball(void)
{
    int cx = ballCenterX;
    int cy = ballCenterY;
    
    // Loop vertically from -3 to +3
    for (int dy = -3; dy <= 3; ++dy) {
        // How wide this row is:
        int span = 3 - abs(dy);      // 3,2,1,0,1,2,3 → widths 0..3
        for (int dx = -span; dx <= span; ++dx) {
            int x = cx + dx;
            int y = cy + dy;
            if (x >= 0 && x < width && y >= 0 && y < height)
                SetPixel(x, y, green);
        }
    }
}


void draw_playing_field()
{

    for (int column = 0; column < NCOLS; ++column){
        for (int row = 0; row < NROWS; ++row){
            if (blocks[column][row].destroyed == 1)
                continue;
            unsigned int color = ((row + column) & 1) ? pink : purple;
            draw_block((unsigned)blocks[column][row].pos_x, (unsigned)blocks[column][row].pos_y, TILE_SIZE, TILE_SIZE, color);
        }
    }

    // draw bar and ball
    draw_bar((unsigned)barY);
    draw_ball();
}





static void reflect_on_block_or_wall(int *pdx, int *pdy,
                                     int cx, int cy, int nx, int ny,
                                     int hit_block_now)
{
    // Axis test: see which axis causes the collision
    int dx_try = *pdx, dy_try = *pdy;

    // test X-only move
    int cx2 = cx + dx_try, cy2 = cy;
    int x0 = leftmost_col_x();

    int hit_x_only  = corner_hits_block(cx2, cy2,x0);

    // test Y-only move
    cx2 = cx; cy2 = cy + dy_try;
    int hit_y_only  = corner_hits_block(cx2, cy2,x0);

    if (hit_x_only && !hit_y_only){ *pdx = -(*pdx); }
    else if (!hit_x_only && hit_y_only){ *pdy = -(*pdy); }
    else { *pdx = -(*pdx); *pdy = -(*pdy); } // corner
}

// destroy any blocks touched by corners at (nx,ny); returns 1 if any destroyed
static int destroy_blocks_at(int nx, int ny)
{
    int x0 = leftmost_col_x();
    int destroyed = 0;
    int pts[4][2] = {{nx, ny-3}, {nx+3, ny}, {nx, ny+3}, {nx-3, ny}};
    for (int k=0;k<4;++k){
        int px = pts[k][0], py = pts[k][1];
        if (px >= x0 && px < 320 && py >= 0 && py < 240){
            int col = (px - x0) / TILE_SIZE;
            int row = py / TILE_SIZE;
            if (row >= 0 && row < 16 && col >= 0 && col < NCOLS && alive[row][col]){
                alive[row][col] = 0;
                destroyed = 1;
            }
        }
    }
    return destroyed;
}

void update_game_state()
{
    if (currentState != Running) return;

    // WIN/LOSE checks (use current center)
    if (ballCenterX + 3 >= (int)width)
    { 
        currentState = Won;
         return;
     }
     //if ball goes past left edge and is not hitting the bar
    if (ballCenterX - 3 < 7 && !(ballCenterY >= barY && ballCenterY <= barY + 45))
    {
        currentState = Lost;
        return;
    }

    // Next intended position, based on angle also
    int nx = ballCenterX + dx;

    }

    // Apply movement
    ballCenterX = nx;
    ballCenterY = ny;

    // Re-check terminal conditions after move
    if (ballCenterX + 3 >= (int)width){ currentState = Won; return; }
    if (ballCenterX - 3 < 7){ currentState = Lost; return; }
}

//move the bar
void update_bar_state()
{
    // Read until no more ready data this tick
    for (;;){
        unsigned long long v = ReadUart();
        if (!uart_ready(v)) return;

        unsigned char ch  = uart_char_typed(v);
        unsigned char rem = uart_remaining(v);

        
        for (int i = -1; i < (int)rem; ++i)
		{
            if(ch == 'w')
			{
				barY -= TILE_SIZE;
				clamp_bar();
				if (currentState == Stopped) currentState = Running; 
			}
            else if (ch == 's')
			{ 
				barY += TILE_SIZE; 
				clamp_bar();
				if (currentState == Stopped) currentState = Running; 
			}
            else if (ch == '\r' || ch == '\n')
			{ 
				currentState = Exit; 
				return; 
			}
 

            if (i + 1 < (int)rem){
                v = ReadUart();
                ch = uart_char_typed(v);
            }
        }
    }
}


void write(char* str)
{
    if (!str) return;
    while (*str) WriteUart(*str++);
    WriteUart('\n');
}

void play()
{
    init_blocks();
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
	
    // This is draining the UART buffer
	
    int remaining = 0;
    do
    {
        unsigned long long out = ReadUart();
        if (!(out & 0x8000))
        {
            return;
        }
      	remaining = (out & 0xFF0000) >> 4;
    } while (remaining > 0);
	
	  // Validate NCOLS
    if (NCOLS < 1 || NCOLS > 18){
        write("Invalid NCOLS (1..18 required).");
        currentState = Exit; // not playable
        return;
    }
	
	
	set_default_values();
    
    
	currentState = Stopped;
}

	

void wait_for_start()
{
    currentState = Stopped;
    while (currentState != Running)
    {
        unsigned long long out = ReadUart();
        if (!uart_ready(out)) continue;
		//character written
        unsigned char ch = uart_char_typed(out);
        if ( (ch == 'w' || ch == 's'))
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
    
    while (1){
        wait_for_start();
        if (currentState == Exit) break;
        set_default_values();
        play();
        reset();
        if (currentState != Running) break;
        if (currentState == Won)  write(won);
        if (currentState == Lost) write(lost);
        if (currentState == Exit) break;
    }
    return 0;
}