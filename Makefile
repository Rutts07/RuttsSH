all: main.c utils.c cmd_list1.c cmd_list2.c bg_procs.c input.c history.c 
	gcc main.c utils.c cmd_list1.c cmd_list2.c bg_procs.c input.c history.c -o shell