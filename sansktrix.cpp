#include <ncurses.h>
#include <locale.h>
#include <vector>
#include <thread>
#include <algorithm>
#include <random>
#include <atomic>
#include <mutex>
#include <iostream>
#include <signal.h>
#include <string>

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

// Single threaded printing flip rate
int flip_rate_ms = 250;

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

// Smooth printing at variables rates per column
// Consumes more resources than single_threaded_print()
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
        x = std::clamp(x + 1, 0, max_row-1);

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

// Consumes less resources but looks clunky
void single_threaded_print()
{
    std::random_device rd;
    std::mt19937 gen(rd());  // Mersenne Twister engine, seeded by random_device
    std::uniform_int_distribution<> depth(1, 3);
    std::vector<const char*> letter(1);
    std::vector<int> x(max_col, 0);

    while (!terminate_thread)
    {

        for (int y = start_col; y<max_col; y += col_step)
        {
            for (int i = 0, j = depth(gen); i < j; i ++)
            {
                // Sample single character
                std::sample(sanskrit_alphabet.begin(), sanskrit_alphabet.end(), letter.begin(), 1, gen);

                // Print character
                {
                    std::lock_guard<std::mutex> lock(screen_mutex);
                    mvprintw(x[y], y, "%s", letter[0]);
                }

                // Next row index
                x[y] = std::clamp(x[y] + 1, 0, max_row-1);

                if (x[y] == max_row-1) // Clear column if last row
                {
                    x[y] = 0;
                    clear_column(y);
                }
            }

        }

        // Sleep for a moment
        std::this_thread::sleep_for(std::chrono::milliseconds(flip_rate_ms));

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

void handle_exit(int sig) {
    endwin(); // Restore terminal before quitting
    _exit(0); // Exit immediately
    sig++; // Avoid unused parameter warning
}

int main(int argc, const char **argv) {
    // Handle exit signals
    signal(SIGINT, handle_exit); // Handle Ctrl+C
    signal(SIGTERM, handle_exit); // Handle termination signals

    // Set locale to support UTF-8
    setlocale(LC_ALL, "");

    // Initialize ncurses
    initscr(); // Start ncurses mode
    cbreak(); // Disable line buffering
    noecho(); // Don't echo user input
    curs_set(0); // Hide the cursor

    // Get screen size
    getmaxyx(stdscr, max_row, max_col);

    // Handle input arguments
    std::vector<std::string> args(argv + 1, argv + argc);

    // Has argument lambda
    auto has = [&args](const std::string& option) {
        return std::find(args.begin(), args.end(), option) != args.end();
    };

    // Create printing threads
    std::vector<std::thread> threads;

    if (has("--single_threaded_print"))
    {
        threads.emplace_back(single_threaded_print);
    }
    else
    {
        for (int i = start_col; i<max_col; i += col_step)
        {
            threads.emplace_back(thread_print, i);
        }
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