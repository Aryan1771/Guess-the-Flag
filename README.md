# Guess the Flag - Field Notes

A graphical Windows C++ flag quiz with a warm, dark atlas interface and genuine flag images loaded from the local asset manifest.

## Run

Double-click `GuessTheFlag.exe`. Keep the adjacent `assets` folder with the executable; the game locates assets relative to its own path, so it can be launched from another working directory. If the manifest is missing, fewer than ten images load, or entries are invalid, the game reports the issue or skips affected entries safely.

## Accounts, scores, and leaderboards

- Create a player account or sign in from the account screen. Existing account names are selectable; accounts can also be paged through.
- Passwords are stored as salted PBKDF2-SHA-256 hashes (210,000 iterations) using Windows CNG. Plaintext passwords are not written to disk.
- New accounts receive a one-time recovery key after account creation. Save it somewhere safe; it can be copied from the screen and is required to reset a forgotten password. Existing accounts receive a key after their next successful sign-in.
- Use **Forgot password** on the sign-in screen or **Change password** from the main menu to set a new password with the account name and recovery key. The recovery key is stored as a salted hash, not plaintext.
- Password and recovery fields have an eye toggle, a blinking insertion caret, and support keyboard cursor navigation (Left/Right, Home/End, Backspace/Delete).
- Account hashes and per-mode score records live in `profiles/` beside the executable. Each account has a `.acct` file and a `.scores` file.
- Standard, Endless, and each Timed length (5, 2, and 1 minute) keep separate high and latest scores. Scores update as you play.
- The **World Leaderboard** is currently a local scoreboard of accounts on this device; it is not online or synchronized between PCs. Open it from the main menu or an end-of-run screen.
- Existing pre-account score data is imported as the account's Standard record when creating a matching account.

## Modes and controls

- **Standard:** ten distinct flags, one final score.
- **Endless:** five wrong answers end the run and show the Game Over score screen. The wrong-answer count is shown while playing; Endless has no Finish or Escape exit.
- **Timed:** race the clock for five, two, or one minute. The quiz ends when time runs out; you can finish early with **Finish Run** or **Esc**.
- Correct answers earn points based on answer speed. Consecutive correct answers increase the streak multiplier; an incorrect answer resets it.
- Flags are drawn without repeats within a run until all loaded countries have been shown.
- Select Standard, Endless, or Timed by clicking its card or pressing **1**, **2**, or **3**. In Timed mode, click a time limit, then press **Enter** to begin.
- Choose an answer by clicking or pressing **1–4**. Advance with **Enter** or the **Next Flag** button.
- From a results screen, open the leaderboard, choose a mode or timer length, then return to the result screen.

## Build and debug in VS Code

Open this folder in VS Code. Use **Ctrl+Shift+B** to build, **F5** to build and debug, or **Ctrl+F5** to build and run without debugging. The `.vscode` directory configures MSYS2 UCRT64 GCC and GDB and compiles the app icon resource before the executable.

## Build from PowerShell

```powershell
windres -i resource.rc -O coff -o icon.o
g++ -std=c++17 -O2 -g -static -static-libgcc -static-libstdc++ -municode -mwindows main.cpp icon.o -o GuessTheFlag.exe -lgdiplus -lgdi32 -luser32 -lbcrypt
```

The compiler is `C:\msys64\ucrt64\bin\g++.exe`. The executable links the GCC C++ runtime statically and embeds its custom flag icon. **Distribute the `assets` folder beside the executable**; flag PNGs load at runtime and are not embedded.

## Flags and country data

- `assets/flags/flags.txt` contains 195 `CODE|COUNTRY_NAME|IMAGE_PATH` records.
- Each country has a local PNG named with its lowercase ISO 3166-1 alpha-2 code.
- The initial collection is the 193 UN member states, Vatican City (Holy See), and Palestine; dependent territories are excluded.
- `assets/flags/ATTRIBUTION.md` and `assets/flags/ODbL-1.0.txt` document the flag-image and country-data sources and their terms.

Add a country by adding its PNG and a manifest line. No source edit or rebuild is needed.
