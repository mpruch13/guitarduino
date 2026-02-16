// Header file “notes.h”
// Custom font that makes up the note sprites displayed on the matrix display
// Must be included in the controller code (save in same folder as the .ino file)
MD_MAX72XX::fontType_t notes[] PROGMEM =
{
    0, // 0 // null
    4, 192, 192, 0, 0, // 1 note-1
    4, 48, 48, 0, 0, // 2 note-2
    4, 12, 12, 0, 0, // 3 note-3
    4, 3, 3, 0, 0, // 4 note-4
    4, 240, 240, 0, 0, // 5 note-1-2
    4, 60, 60, 0, 0, // 6 note-2-3
    4, 15, 15, 0, 0, // 7 note-3-4
    4, 252, 252, 0, 0, // 8 note-1-2-3
    4, 63, 63, 0, 0, // 9 note-2-3-4
    4, 255, 255, 0, 0, // 10 note-1-2-3-4
    4, 204, 204, 0, 0, // 11 note-1-3
    4, 195, 195, 0, 0, // 12 note-1-4
    4, 51, 51, 0, 0, // 13 note-2-4
    4, 243, 243, 0, 0, // 14 note-1-2-4
    4, 207, 207, 0, 0, // 15 note-1-3-4
    4, 0, 0, 0, 0, // 16 blank
    // Can have up to 256 chars/sprites, but we only need these 17 for now
} // End of “notes.h”