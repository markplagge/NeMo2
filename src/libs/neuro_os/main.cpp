//
// Created by Mark Plagge on 6/20/20.
//
#include "include/neuro_os.h"

int main(){
	auto p = std::make_shared<neuro_os::sim_proc::SimProc>();
	neuro_os::sim_proc::SimProcessQueue q;
	q.enqueue(p);
	return 0;
}