#pragma once

#define RESET "\033[0m"
#define BOLD "\033[1m"
#define DIM "\033[2m"
#define ITALIC "\033[3m"
#define UNDERLINE "\033[4m"
#define BLINK "\033[5m"
#define INVERSE "\033[7m"
#define STRIKETHROUGH "\033[9m"

#define BLACK "\033[38;5;0m"
#define RED "\033[38;5;1m"
#define GREEN "\033[38;5;2m"
#define YELLOW "\033[38;5;3m"
#define BLUE "\033[38;5;4m"
#define MAGENTA "\033[38;5;5m"
#define CYAN "\033[38;5;6m"
#define WHITE "\033[38;5;7m"
#define GRAY "\033[38;5;8m"

#define BRIGHT_RED "\033[38;5;9m"
#define BRIGHT_GREEN "\033[38;5;10m"
#define BRIGHT_YELLOW "\033[38;5;11m"
#define BRIGHT_BLUE "\033[38;5;12m"
#define BRIGHT_MAGENTA "\033[38;5;13m"
#define BRIGHT_CYAN "\033[38;5;14m"
#define BRIGHT_WHITE "\033[38;5;15m"

#define DEBUG GRAY
#define INFO WHITE
#define WARNING YELLOW
#define ERROR RED