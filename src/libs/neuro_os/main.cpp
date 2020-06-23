//
// Created by Mark Plagge on 6/20/20.
//
#include "include/neuro_os.h"

int main(){
	neuro_os::sim_proc::SimProc p;
	neuro_os::sim_proc::SimProcessQueue q;
	q.enqueue(p);
	return 0;
}