// // use a singleton for this
// // singleton class insures that only one instance of the class can be created  
// // typical use case is for global functionality, like this case

// #pragma once

// #include <chrono>
// #include <fstream>

// class Timer
// {
//     std:ofstream
//     long long m_start = 0, m_end = 0;
//     std::chrono::time_point<std::chrono::high_resolution_clock> m_stp;

// public:
//     Timer() { start(); }
//     void start() { m_stp = std::chrono::high_resolution_clock::now(); }
//     long long elapsed() // returns elapsed time in microseconds
//     {
//         using namespace std::chrono;
//         auto etp = high_resolution_clock::now();
//         m_start = time_point_cast<microseconds>(m_stp).time_since_epoch().count();
//         m_end = time_point_cast<microseconds>(etp).time_since_epoch().count();
//         return m_end - m_start;
//     }
// }