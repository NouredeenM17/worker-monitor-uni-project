
all: worker_monitor adder subtractor multiplier divider

worker_monitor: worker_monitor.c
	gcc worker_monitor.c -o worker_monitor

adder: adder.c
	gcc adder.c -o adder

subtractor: subtractor.c
	gcc subtractor.c -o subtractor

multiplier: multiplier.c
	gcc multiplier.c -o multiplier

divider: divider.c
	gcc divider.c -o divider
