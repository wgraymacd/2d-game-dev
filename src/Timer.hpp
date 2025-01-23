// use a singleton for this
// singleton class insures that only one instance of the class can be created  
// typical use case is for global functionality, like this case

#define PROFILING
#ifdef PROFILING
#define PROFILE_SCOPE(name) \
        ProfileTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() \
        PROFILE_SCOPE(__FUNCTION__)
#else
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#endif

#pragma once

#include <string>
#include <chrono>
#include <fstream>
#include <thread>

struct ProfileResult
{
    std::string name = "Default";
    long long start = 0;
    long long end = 0;
    size_t threadID = 0;
};

class Profiler
{
    std::ofstream m_fout = std::ofstream("result.json");
    bool m_firstEntry = true;

    Profiler()
    {
        m_fout << "{\n    \"traceEvents\":\n    [\n";
    }

public:
    static Profiler& Instance()
    {
        static Profiler instance;
        return instance;
    }
    void writeProfile(const ProfileResult& r)
    {
        if (!m_firstEntry)
        {
            m_fout << ",\n";
        }
        else
        {
            m_firstEntry = false;
        }

        m_fout << "        { "
            << "\"name\": \"" << r.name << "\", "
            << "\"cat\": \"\", "
            << "\"ph\": \"X\", "
            << "\"ts\": " << r.start << ", "
            << "\"dur\": " << (r.end - r.start) << ", "
            << "\"pid\": 0, "
            << "\"tid\": " << r.threadID << " }";
    }

    ~Profiler()
    {
        // flush();
        m_fout << "\n    ]\n}";
        m_fout.close();
    }
}; // singleton will destruct when program exists normally, when Timer stops, call Profiler::Instance().writeProfile() 

class ProfileTimer
{
    ProfileResult m_result;
    bool m_stopped = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_stp;

    void stop()
    {
        using namespace std::chrono;

        if (m_stopped) { return; }
        m_stopped = true;

        auto etp = high_resolution_clock::now();
        m_result.start = time_point_cast<microseconds>(m_stp).time_since_epoch().count();
        m_result.end = time_point_cast<microseconds>(etp).time_since_epoch().count();
        m_result.threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());

        Profiler::Instance().writeProfile(m_result);
    }

public:
    ProfileTimer(const std::string& name)
    {
        m_result.name = name;
        m_stp = std::chrono::high_resolution_clock::now();
    }
    ~ProfileTimer() { stop(); }
};