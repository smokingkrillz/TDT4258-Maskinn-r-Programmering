// palinfinder.s, Lab1 TDT4258 autumn 2025
.global _start

// ===== Memory-mapped I/O =====
//writing to 0xFF200000 will turn on the LEDs taken from pdf provided
.equ LEDR_BASE,      0xFF200000
//writing to 0xFF201000 will write a char to the UART
.equ UART0_DATA,     0xFF201000
//reading from 0xFF201004 will read the UART control register
.equ UART0_CONTROL,  0xFF201004

// 
_start:
    // Clear LEDs before starting new run
    LDR     r1, =LEDR_BASE
    MOV     r0, #0
    STR     r0, [r1]

    // Load input string address into r0
    LDR     r0, =input

    // Branch with link to check_input function
    // this means we save what the fun returns, r0
    BL      check_input
    CMP     r0, #0
    BEQ     is_not_palindrom

    // If input length >= 2, continue with palindrome check
    // Reload input address for palindrome check
    LDR     r0, = input
    BL      check_palindrom 
    CMP     r0, #0
    // check if return value is equal to 0
    // if true, then its not a palindrome, we call that function
    BEQ     is_not_palindrom
    B       is_palindrom

// -------------------------------------------------------------
// check_input:
// - You could use this symbol to check for your input length
// - You can assume that your input string is at least 2 characters 
//   long and ends with a null byte
// - r0 = &input, return r0 = 1 if length >= 2, else 0
// -------------------------------------------------------------
check_input:
    // load register byte
    
    LDRB    r1, [r0]          // load first char
    CMP     r1, #0
    BEQ     too_short          // "" is too short
    LDRB    r1, [r0, #1]       // load second char
    CMP     r1, #0
    BEQ     too_short          // "x" is too short
    MOV     r0, #1             // valid length
    BX      lr
too_short:
    // if we are here, it means length < 2
    MOV     r0, #0
    BX      lr

// -------------------------------------------------------------
// check_palindrom:
// - Main palindrome checking function
// - Returns r0 = 1 if palindrome, 0 otherwise
// -------------------------------------------------------------
check_palindrom:
    // push is saving registers on stack
    PUSH    {r4-r7, lr}
    
    // r4 is left pointer to go through input, r5 is right
    // both are equal to the start char of input
    MOV     r4, r0
    MOV     r5, r0

// find end of string
find_end_loop:
    LDRB    r1, [r5]          // r1 is now equal to the value stored at r5
    CMP     r1, #0            // is it end of string?
    BEQ     found_end
    ADD     r5, r5, #1
    B       find_end_loop

found_end:
    SUB     r5, r5, #1        // now r5 points to the last char

// -------------------------------------------------------------
// Main loop: skip spaces, normalize chars, compare left & right
// -------------------------------------------------------------
main_loop:

// spaces have to be skipped, we take left pointer first
skip_left_spaces:
    LDRB    r1, [r4]
    CMP     r1, #' '          // we compare char to space
    BNE     check_right_side  // if not equal we go forward
    ADD     r4, r4, #1        // check next char
    B       skip_left_spaces  // go do left skip loop again

// right pointer
check_right_side:
    LDRB    r2, [r5]
    CMP     r2, #' '
    BNE     compare_pointers
    SUB     r5, r5, #1 
    B       skip_left_spaces 

// lowercase check for left char
compare_pointers:
    // if left pointer is higher or equal to right, we are done
    CMP     r4, r5
    BHS     palindrome_success
    
    // Manual lowercase: if 'A'..'Z' add 32
    MOV     r3, r1
    CMP     r3, #'A'
    BLT     skip_left_lower    // if lesser than we skip as it is not uppercase
    CMP     r3, #'Z'
    BGT     skip_left_lower
    ADD     r1, r1, #32        // we add 32 and convert to lower if it is less than Z
skip_left_lower:

// same for right char
    MOV     r3, r2
    CMP     r3, #'A'
    BLT     skip_right_lower
    CMP     r3, #'Z'
    BGT     skip_right_lower
    ADD     r2, r2, #32
skip_right_lower:

// compare chars
    // if equal, advance both
    CMP     r1, r2
    BEQ     advance_pointers
    
    // Wildcards: if ? or # then it is the same as the char
    CMP     r1, #'?'
    BEQ     advance_pointers
    CMP     r1, #'#'
    BEQ     advance_pointers
    CMP     r2, #'?'
    BEQ     advance_pointers
    CMP     r2, #'#'
    BEQ     advance_pointers
    
    // if the code is here it means it is not equal, so we
    // end the script and send false
    MOV     r0, #0
    B       palindrome_done

advance_pointers:
    // go to next char and compare
    ADD     r4, r4, #1
    SUB     r5, r5, #1
    B       main_loop          // go to main loop and compare again

palindrome_success:
    // if we are here, it means we have a palindrome
    MOV     r0, #1

palindrome_done:
    // pop registers and return, pc means return
    POP     {r4-r7, pc}    

// -------------------------------------------------------------
// is_palindrom: LEDs + UART message
// - Switch on only the 5 rightmost LEDs (bits 0..4 = 1 -> 0x1F)
// - Write "Palindrome detected" to UART
// -------------------------------------------------------------
is_palindrom:
    // LED = 0x0000001F
    LDR     r1, =LEDR_BASE
    MOV     r0, #0b0000011111  // sending 1s to the 5 rightmost LEDs
    STR     r0, [r1]
    
    // UART print
    LDR     r0, =displayPalindrome
    BL      uart_print_string
    // branch to exit
    B       _exit

// -------------------------------------------------------------
// is_not_palindrom: LEDs + UART message
// - Switch on only the 5 leftmost LEDs (bits 9..5 -> 0x3E0)
// - Write "Not a palindrome" to UART
// -------------------------------------------------------------
is_not_palindrom:
    // LED = 0x000003E0
    LDR     r1, =LEDR_BASE
    MOV     r0, #0b1111100000  // sending 1s to the 5 leftmost LEDs
    STR     r0, [r1]
    
    // UART print
    LDR     r0, =displayNotPalindrome
    BL      uart_print_string
    B       _exit

// -------------------------------------------------------------
// uart_print_string:
// - Function to print string to UART
// - Input: r0 = address of string
// -------------------------------------------------------------
uart_print_string:
    // save registers and link register
    PUSH    {r4, lr}
    MOV     r4, r0
print_loop:
    // load byte from address in r4 to r0
    LDRB    r0, [r4] 
    CMP     r0, #0             // check for nullbyte
    // if null, we are done
    BEQ     print_done
    // if not null, we call putc_uart to print char
    BL      putc_uart
    // advance to next char
    ADD     r4, r4, #1
    B       print_loop
print_done:
    // pop registers and return
    POP     {r4, pc}

// -------------------------------------------------------------
// putc_uart:
// - Function to print single character
// - Input: r0 = char to print
// - Wait until UART is ready to transmit 
// -------------------------------------------------------------
putc_uart:
wait_for_space:
    // 1) Poll control register until there is write space (WSPACE > 0)
    LDR     r1, =UART0_CONTROL   // get the address of the UART control/status register
    LDR     r2, [r1]             // read the 32-bit control value
    // this means we shift right 16 times, so we get the WSPACE value
    // WSPACE is bits [31:16], 16 times because we want the most significant bits
    // in the least significant bits position because we want to compare it to 0
    LSRS    r2, r2, #16          // r2 = WSPACE (bits [31:16] >> 16)
    BEQ     wait_for_space       // if WSPACE == 0, keep waiting

    // 2) Write character to data register
    LDR     r1, =UART0_DATA      // r1 = &UART0_DATA (0xFF201000)
    STR     r0, [r1]             // send r0; hardware uses r0[7:0]

    // 3) Return to caller
    BX      lr

// -------------------------------------------------------------
// Exit: infinite loop
// -------------------------------------------------------------
_exit:
    // Branch here for exit
    B       .

// ===== Data section =====
.data
.align
// are not allowed to change the name 'input'!
input: .asciz "step on no pets"

// This is the message that the UART will display in each case
displayNotPalindrome: .asciz "Not a palindrome\r\n"
displayPalindrome:    .asciz "Palindrome detected\r\n"

.end

// AI Statement : I have used Github copilot to explain parts of the code that I took directly from the pdf
// provided to interact with LED and UART Board, as this was not intuitive for me. 
//I struggled a bit to understand WSPACE, and I asked copilot to explain this text line by line
// so that I could understand much easier.
// Copilot also advised me to divide the code into loops with useful names.