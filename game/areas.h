// Area definitions - Simple array-based maps
// =----------------------------------------=
// Each playfield is made up of 18x10 tiles

#ifndef AREAS_H
#define AREAS_H

// Area tag structure
struct area_t {
    unsigned char* ptr; // Pointer to area definition
    
    // These flags determine what's necessary to progress to the left or right.
    // The game will look for any objects of the corresponding nature and block
    // moving over to the next or previous screen if any exist

    char killAllEnemies, killAllGrass;
    char rightHitSwitch, leftHitSwitch;
    char rightInnerSwitch;
    char leftInnerSwitch;
    
    unsigned char* inner; // Pointer to 'inner' area if it exists   
    unsigned char innerShop; 
    unsigned char treasure;
};

// Area definitions
#define AREA_SIZE (10*18)

unsigned char template_area[AREA_SIZE] = {
    'C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C',    
    'c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c',    
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '        
};

// LEGEND:
// C - cliff top
// c - cliff connecting to grass
// # - grass/bush
// . - stump
// @ - rock
// X - bomb-able wall
// D - open door
// e - enemy type 1
// ? - treasure chest
// $ - shop object

unsigned char a1_1start[AREA_SIZE] = {
    'C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C',    
    'c','c','c','c','c','c','c','c','D','c','c','c','c','c','c','c','c','c',    
    '#','#','@',' ',' ',' ','#','#',' ','#','#',' ',' ',' ','@',' ',' ',' ',        
    '#','#',' ',' ',' ',' ','#','#',' ','#','#',' ','e',' ','@',' ','?',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#','#',' ',' ','@',' ',' ',' ',        
    '#','#','#','#',' ',' ','e',' ',' ',' ','#','#','#','#','@','@','@','@',        
    '#','#','#',' ','#','#','#','#','#','#','#','#','#','#',' ',' ',' ',' ',        
    ' ',' ',' ',' ','.','#','#','#','#','#','#','#','#',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '
};

unsigned char a1_1inner[AREA_SIZE] = {
    'C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C',    
    'C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C',    
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ','$',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '        
};

unsigned char a1_2[AREA_SIZE] = {
    'C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C',    
    'c','C','C','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c',    
    ' ','c','c',' ',' ','@','#','#','#','#','@','@',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ','@','#',' ',' ','#','@','@',' ',' ',' ','!',' ',' ',        
    ' ',' ',' ',' ',' ','@','#','e',' ','#','@',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ','@','#',' ',' ','#','@','#','#','#','#','#','#','#',        
    ' ',' ',' ',' ',' ','@','#','#','#','#','@','#',' ','#','#','#','#','#',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',        
    '@',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','@' 
};
// The Area List contains structures to pointers to the actual area layouts and flags. It's cyclical, so if you
// hit the last room and scroll around, it'll load the first room in the list.

#define NUM_AREAS 2

struct area_t areas[NUM_AREAS] = {
    // ptr, killAllEnemies, killAllGrass, rightSwitch, leftSwitch, rightInnerSwitch, leftInnerSwitch, innerptr, innerIsShop
    {a1_1start, 1, 0, 0, 0, 1, 0, a1_1inner, 1, 0},
    {a1_2, 1, 0, 0, 0, 1, 0, 0, 1, 1}
};

#endif