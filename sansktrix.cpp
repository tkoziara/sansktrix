#include <ncurses.h>
#include <locale.h>
#include <vector>
#include <thread>
#include <algorithm>
#include <random>
#include <atomic>
#include <mutex>
#include <iostream>

// Devanagari letters representing the Sanskrit alphabet
std::vector<const char*> sanskrit_alphabet = {
    "अ", "आ", "इ", "ई", "उ", "ऊ", "ऋ", "ॠ", "ऌ", "ॡ", "ए", "ऐ", "ओ", "औ",
    "क", "ख", "ग", "घ", "ङ",
    "च", "छ", "ज", "झ", "ञ",
    "ट", "ठ", "ड", "ढ", "ण",
    "त", "थ", "द", "ध", "न",
    "प", "फ", "ब", "भ", "म",
    "य", "र", "ल", "व",
    "श", "ष", "स", "ह",
    "ळ", "क्ष", "ज्ञ"
};

// First column to print
int start_col = 1;

// Column step
int col_step = 4;

// Screen size
int max_row, max_col;

// Atomic flag to control thread termination
std::atomic<bool> terminate_thread(false);

// Screen buffer access mutex
std::mutex screen_mutex;

// Clear vertical screen column
void clear_column(int y) {
    std::lock_guard<std::mutex> lock(screen_mutex);
    for (int x = 0; x < max_row; ++x) {
        for (int i = -1; i <= 1; ++i) {
            if (y+i >= 0 && y+i < max_col) {
                mvprintw(x, y+i, " "); // Overwrite with space
            }
        }
    }
}

void thread_print(int y)
{
    std::random_device rd;
    std::mt19937 gen(rd());  // Mersenne Twister engine, seeded by random_device
    std::uniform_int_distribution<> dist(50, 500);
    std::vector<const char*> letter(1);
    thread_local int x = 0;

    while (!terminate_thread)
    {
        // Sample single character
        std::sample(sanskrit_alphabet.begin(), sanskrit_alphabet.end(), letter.begin(), 1, gen);

        // Print character
        {
            std::lock_guard<std::mutex> lock(screen_mutex);
            mvprintw(x, y, "%s", letter[0]);
        }

        // Next row index
        x = std::clamp(x + 1, -1, max_row-1);

        if (x == max_row-1) // Clear column if last row
        {
            x = 0;
            clear_column(y);
        }

        // Sleep between 50 and 500 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));

        // Refresh screen
        {
            std::lock_guard<std::mutex> lock(screen_mutex);
            refresh();
        }
    }
}

void press_any_key_thread()
{
    while (!terminate_thread)
    {
        int ch = getch();

        if (ch != ERR) {  // A key has been pressed
            terminate_thread = true;  // Set flag to terminate the thread
            break;
        }
    }
}

int main() {
    // Set locale to support UTF-8
    setlocale(LC_ALL, "");

    // Initialize ncurses
    initscr();
    noecho();
    curs_set(0);  // Hide cursor

    // Get screen size
    getmaxyx(stdscr, max_row, max_col);

    // Create printing threads
    std::vector<std::thread> threads;

    for (int i = start_col; i<max_col; i += col_step)
    {
        threads.emplace_back(thread_print, i);
    }

    threads.emplace_back(press_any_key_thread);

    // Join all threads
    for (auto& t : threads)
    {
        t.join();
    }

    // End ncurses mode
    endwin();

    return 0;
}