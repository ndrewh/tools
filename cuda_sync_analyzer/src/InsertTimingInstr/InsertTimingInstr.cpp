#include <chrono>
#include <map>
#include <vector>
#include <stack>
#include <cassert>
#include <iostream>
#include <fstream>
#include <memory>
#include <cstdlib>

typedef std::chrono::high_resolution_clock hrc;
std::chrono::time_point<hrc> default_time = hrc::now();

// Struct { start, end, id };

// vector<Structs> times;

// vector<Structs> unresoived;
// map<id, vector<struct> > unresolved;
// unordered_map<id, vector<struct>> unresolved;

// Entry() { unresolved.push_back(start, 0, id) };

// Exit() { for(int i = unresolved.size() - 1; ....; i--){if(unresolved[i].id == id) add end time, close)}


struct ExecTime {
    uint64_t id;
    hrc::time_point start_time;
    hrc::time_point end_time;
};

std::vector<ExecTime> times;
std::stack<ExecTime> unresolved;

extern "C" {
    void SAVE_INSTR_TIMES() {
        //std::ofstream out("InstrTimings.out");
        //assert(out.good());
        for (std::vector<ExecTime>::iterator it = times.begin(); it != times.end(); ++it)
            std::cout << it->id << ":"
                << std::chrono::duration<double, std::milli>(it->end_time - it->start_time).count()
                << std::endl;
        //out.close();
    }
    
    void SignalStartInstra() {
        if (atexit(SAVE_INSTR_TIMES) != 0)
            std::cerr << "Failed to register atexit function" << std::endl;
    }
    void CALL_ENTRY(uint64_t id) {
        struct ExecTime time;
        time.id = id;
        unresolved.push(time);
        auto start = hrc::now();
        unresolved.top().start_time = start;
    }
    void CALL_EXIT(uint64_t id) {
        auto stop = hrc::now();
        struct ExecTime time = unresolved.top();
        time.end_time = stop;
        unresolved.pop();
        times.push_back(time);
    }
}