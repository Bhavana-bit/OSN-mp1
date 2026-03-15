#ifndef B2_H
#define B2_H

// Implements the "reveal" command (ls-like)
// input: full command string from user
// prev: previous directory (for "reveal -")
void handle_reveal(char *input, char *prev);

#endif
