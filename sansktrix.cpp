#include <ncurses.h>
#include <locale.h>
#include <vector>
#include <thread>
#include <algorithm>
#include <random>
#include <atomic>

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

// Screen size
int max_row, max_col;

// Atomic flag to control thread termination
std::atomic<bool> terminate_thread(false);

void thread_print(int y)
{
    std::random_device rd;
    std::mt19937 gen(rd());  // Mersenne Twister engine, seeded by random_device
    std::uniform_int_distribution<> dist(50, 500);
    std::vector<const char*> letter(1);
    thread_local int col_length = -1;

    while (!terminate_thread)
    {
        // Next row index
        int x = col_length + 1;

        // Sample single character
        std::sample(sanskrit_alphabet.begin(), sanskrit_alphabet.end(), letter.begin(), 1, gen);

        mvprintw(x, y, "%s", letter[0]);

        if (x > max_row) x = -1;

        col_length = x;

        // Sleep between 50 and 500 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));

        refresh();  // Refresh screen to show output

        if (y == 0) // Check if a key has been pressed
        {
            int ch = getch();

            if (ch != ERR) {  // A key has been pressed
                terminate_thread = true;  // Set flag to terminate the thread
                break;
            }
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
    std::vector<std::thread> printing_threads;

    for (int i = 0; i<max_col; i += 2)
    {
        printing_threads.emplace_back(thread_print, i);
    }

    // Join all threads
    for (auto& t : printing_threads)
    {
        t.join();
    }

    // End ncurses mode
    endwin();

    return 0;
}
