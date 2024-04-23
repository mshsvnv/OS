NUM=2
PID=28243

start_clients:
	./start_clients.sh 32 ./lab_0$(NUM)/

pagemap:
	./pmap.sh $(PID) $(NUM)

proc_info:
	./proc/proc_info $(PID)
	mv output.txt output_lab_0$(NUM).txt
	rm output_pagemap.txt