# RuttsSH
This repository contains the code for a custom shell made with C. This shell supports the following commands :
    [x] history
    [x] pwd
    [x] echo [arg ...]
    [x] cd [.] [~] [..] [-] [arg ...]
    [x] ls [.] [~] [..] [- a | l | al | la] [arg ...]
    [x] discover [.] [~] [..] [- d | f | df | fd] ['filename']
    [x] pinfo [pid]
    [x] jobs [-r | -s | -rs | -sr]
    [x] fg
    [x] bg
    [x] sig [pid] [sig_num]
    [x] clear
    [x] history [-c]
    [x] help
    [x] exit

Custom features :
    [x] piping (|) and I/O redirection (<, >, >>)
    [x] signal handling for Ctrl+C, Ctrl+Z and Ctrl+D
    [x] auto-completion (using 'TAB' key)
    [x] auto-suggestion (using 'CTRL+N' key)
    [x] loading previous commands (using 'UP' & 'DOWN' arrow key)
    [x] help (to list all the commands supported by this shell)
    [x] discover (to recursively search / list all files in a directory)
    [x] jobs (to list all background processes)
    [x] sig (to send signals to background processes)
    [x] fg (to bring background process to foreground)
    [x] bg (to resume background process)
    [x] history (to list all the commands executed in the shell, -c flag to clear history)

---

### File Structure
    - header.h
        - Contains all the function definitions and global variable declarations which was used across various files

    - main.c
        - Contains the command prompt code which loops in a while loop
        - Also contains signal handling for Ctrl+C, Ctrl+Z and Ctrl+D key entered

    - cmd_list1.c
        - Contains the code for the implementation of the following :
            [x] pwd
            [x] echo
            [x] cd
            [x] help
            [x] executing foreground processes
            [x] executing background processes
            [x] killing background processes
            [x] I/O redirection
            [x] Piping

    - cmd_list2.c
        - Contains the code for the implementation of the following :
            [x] ls
            [x] pinfo
            [x] discover

    - bg_procs.c
        - Contains the code for manipulating background processes
            [x] jobs
            [x] sig
            [x] fg
            [x] bg

    - input.c
        - Modified boilerplate code for getting input using termios.h

    - history.c
        - Contains the code for history command 

    - utils.c
        - Contains basic code for string manipulations. directory sorting and command auto suggestion
    
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

    - Auto-completion & Auto-suggestion :
        - Autocompletion was done using <termios.h>, rawmode functions given in the boilerplate code. Auto completed files and directories printed out in light grey colour. Auto-completion using 'TAB' ('/t')
        - Autosuggestion was also done using <termios.h>, rawmode functions given in the boilerplate code. Auto suggested commands printed out in light grey colour. Auto-suggestion using 'CTRL+N' ('^N')
        - Previous command updated in the history which can be re-printed in the command prompt by pressing 'UP' arrow key once.

    - Colour coding & Error Handling :
        - Appropiate colour coding was done using the ANSI codes for command prompts, directories, executables and time taken for foreground processes
        - Appropriate error handling was done for all the commands using printf statements in stdout
    
#### Execution 
    - Use the command `make all` to generate the object file ('shell')
    - Run `./shell` to run the shell in your terminal
