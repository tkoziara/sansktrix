# Sansktrix
Sanskrit-Themed Matrix Screensaver for Your Linux/Unix Terminal – Watch Devanagari Characters Flow Like Digital Rain!

![Sanskrit-Themed Matrix Screensaver](screenshot.png)

# Compile

Install nscurses devel files (e.g. `sudo apt-get install libncurses-dev` on Debian based Linux distros). Make sure that you have g++ in the prompt. Clone the repository and from within it run:

```bash
./compile
```

# Set-up

Edit the header section of the `sansktrix_daemon.sh` within the repository

```bash
# Path to sansktrix executable
SANSKTRIX_PATH="$HOME/sansktrix/sansktrix"

# Default inactivity timeout in seconds
INACTIVITY_TIMEOUT=60 
```

and add this line to your .bashrc file

```bash
# Start sansktrix daemon
$HOME/sansktrix/sansktrix_daemon.sh
```

You can test the script first by running it by hand and changing idle timeout and observing logs:

```bash
./sansktrix_daemon.sh
```

You can also kill the daemon at any time by calling:

```bash
./sansktrix_kill.sh
```

# Thanks

Many thanks to the authors of ChatGPT, DeepSeek, Claude Code, and GitHub's Copilot, all of which have greatly contributed to sketching and debugging this code!

# Contribute

Feel free to contribute bug fixes and improvements.