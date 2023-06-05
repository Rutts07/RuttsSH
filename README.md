# RuttsSH
This repository contains the code for a custom shell made with C. This shell supports the following commands :
- history
- pwd
- echo [arg ...]
- cd [.] [~] [..] [-] [arg ...]
- ls [.] [~] [..] [- a | l | al | la] [arg ...]
- discover [.] [~] [..] [- d | f | df | fd] ['filename']
- pinfo [pid]
- jobs [-r | -s | -rs | -sr]
- fg
- bg
- sig [pid] [sig_num]
- clear
- history [-c]
- help
- exit

Custom features :
- piping (|) and I/O redirection (<, >, >>)
- signal handling for Ctrl+C, Ctrl+Z and Ctrl+D
- auto-completion (using 'TAB' key)
- auto-suggestion (using 'CTRL+N' key)
- loading previous commands (using 'UP' & 'DOWN' arrow key)
- help (to list all the commands supported by this shell)
- discover (to recursively search / list all files in a directory)
- jobs (to list all background processes)
- sig (to send signals to background processes)
- fg (to bring background process to foreground)
- bg (to resume background process)
- history (to list all the commands executed in the shell, -c flag to clear history)
---

### Execution 
- Use the command `make all` to generate the object file ('shell')
- Run `./shell` to run the shell in your terminal

--- 
### Specifications :
- I/O Redirection : 
    -Input / Output redirection was done using dup2 sys calls, where in the input ('<') output ('>' and '>>') files were duplicated with STDIN_FILENO and STDOUT_FILENO respectively.

- Piping :
    - Multiple pipes were handled using array of pipes and forking those processes as child processes. I/O was also changed to read end of previous pipe and write end of current pipe respectively for those child processes

- Jobs :
    - jobs command implemented with flags `-r` | `-s` | `-rs` | `-sr`, if no flags are given, default `-rs` is taken
    - fg, bg and sig commands were implemented

- Signal Handling :
    - CTRL+C was handled using signal `SIG_INT`
    - CTRL+Z was handled using signal `SIGTSTP` - Pls Note while using ```make run``` command, the shell is a foreground process for make, hence on pressing CTRL+Z, the shell terminates. Pls use ```./shell``` to test the CTRL+Z handling.
    - CTRL+D was handled using rawmode()

- Auto-Completion & Auto-Suggestion :
    - Autocompletion was done using <termios.h>, rawmode functions given in the boilerplate code. Auto completed files and directories printed out in light grey colour. Auto-completion using 'TAB' ('/t')
    - Autosuggestion was also done using <termios.h>, rawmode functions given in the boilerplate code. Auto suggested commands printed out in light grey colour. Auto-suggestion using 'CTRL+N' ('^N')
    - Previous command updated in the history which can be re-printed in the command prompt by pressing 'UP' arrow key once.

- Colour Coding & Error Handling :
    - Appropiate colour coding was done using the ANSI codes for command prompts, directories, executables and time taken for foreground processes
    - Appropriate error handling was done for all the commands using printf statements in stdout and stderr
