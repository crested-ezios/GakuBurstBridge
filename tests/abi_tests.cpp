#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <thread>
#include <string>
#include <fstream>
#include "GakuBurstBridge.h"
extern "C" int gbb_c_layout(void);
#define CHECK(expr) do { if (!(expr)) { std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#expr); return 1; } } while (0)
int main(int argc, char** argv) {
    CHECK(argc == 2 && gbb_c_layout());
    HMODULE dll = LoadLibraryExA(argv[1], nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
    CHECK(dll);
    auto get = reinterpret_cast<GbbGetApiFn>(GetProcAddress(dll, "GakuBurstBridge_GetApi")); CHECK(get);
    CHECK(get(GBB_API_VERSION, nullptr) == GBB_E_ARGUMENT);
    GbbApi api{}; CHECK(get(GBB_API_VERSION, &api) == GBB_E_ARGUMENT);
    api.size = sizeof(api); api.version = GBB_API_VERSION;
    CHECK(get(123, &api) == GBB_E_VERSION);
    CHECK(get(GBB_API_VERSION, &api) == GBB_OK);
    CHECK(api.publish && api.configure && api.clear && api.shutdown && api.get_status && api.get_perf && api.dump_traces);
    CHECK(get(1, &api) == GBB_E_VERSION);
    CHECK(api.configure(nullptr) == GBB_E_ARGUMENT);
    GbbConfig config{}; config.size = sizeof(config); config.version = GBB_API_VERSION;
    config.reserved[0] = 1; CHECK(api.configure(&config) == GBB_E_ARGUMENT);
    config.reserved[0] = 0; CHECK(api.configure(&config) == GBB_OK);
    GbbStatus status{}; status.size = sizeof(status); status.version = GBB_API_VERSION;
    CHECK(api.get_status(&status) == GBB_OK && status.state == GBB_STATE_UNINITIALIZED);
    CHECK(api.publish(nullptr) == GBB_E_ARGUMENT);
    GbbFrame frame{}; frame.size = sizeof(frame); frame.version = GBB_API_VERSION;
    frame.actor = reinterpret_cast<void*>(1); frame.animator = reinterpret_cast<void*>(2); frame.bone_count = 2;
    frame.bones[0].transform = reinterpret_cast<void*>(3); frame.bones[0].humanoid_id = 0; frame.bones[0].local_rotation.w = 1;
    frame.bones[1].transform = reinterpret_cast<void*>(4); frame.bones[1].humanoid_id = 10; frame.bones[1].local_rotation.w = 1;
    // Without GameAssembly, a well-formed frame must fail before dereferencing Actor pointers.
    CHECK(api.publish(&frame) == GBB_E_UNAVAILABLE);
    int otherThreadResult{}; std::thread t([&]{ otherThreadResult = api.publish(&frame); }); t.join();
    CHECK(otherThreadResult == GBB_E_THREAD);
    CHECK(api.get_status(&status) == GBB_OK && status.state == GBB_STATE_UNAVAILABLE && status.sequence == 0);
    GbbPerf perf{}; perf.size = sizeof(perf); perf.version = GBB_API_VERSION;
    CHECK(api.get_perf(&perf) == GBB_OK && perf.publish_calls == 3 && perf.qpc_frequency > 0);
    CHECK(perf.solver_calls == 0 && perf.native_passes == 0 && perf.delta_probe_calls == 0);
    CHECK(api.clear(2) == GBB_E_ARGUMENT);
    CHECK(api.clear(GBB_CLEAR_RESET_FAULT) == GBB_OK);
    CHECK(api.get_status(&status) == GBB_OK && status.state == GBB_STATE_UNAVAILABLE);
    CHECK(api.dump_traces(nullptr) == GBB_E_ARGUMENT && api.dump_traces(L"relative.json") == GBB_E_ARGUMENT);
    wchar_t temporary[MAX_PATH]; CHECK(GetTempPathW(MAX_PATH, temporary));
    std::wstring tracePath = std::wstring(temporary) + L"GakuBurstBridge-abi-traces.json";
    CHECK(api.dump_traces(tracePath.c_str()) == GBB_OK);
    std::ifstream traceFile{tracePath.c_str()}; std::string traceJson((std::istreambuf_iterator<char>(traceFile)), {}); traceFile.close();
    CHECK(traceJson.find("\"api_version\": 2") != std::string::npos && traceJson.find("\"trace_capacity\": 32") != std::string::npos);
    CHECK(traceJson.find("\"metadata_report_available\": false") != std::string::npos);
    CHECK(traceJson.find("\"observed\": {\"pointer_size\":8}") != std::string::npos);
    CHECK(DeleteFileW(tracePath.c_str()));
    std::thread shutdownThread([&]{ otherThreadResult = api.shutdown(); }); shutdownThread.join();
    CHECK(otherThreadResult == GBB_OK && api.shutdown() == GBB_OK);
    CHECK(api.publish(&frame) == GBB_E_SHUTDOWN);
    CHECK(api.configure(&config) == GBB_E_SHUTDOWN && api.clear(0) == GBB_E_SHUTDOWN);
    CHECK(api.get_status(&status) == GBB_OK && status.state == GBB_STATE_SHUTDOWN);
    CHECK(FreeLibrary(dll));
    std::puts("ABI tests: C layout, version/size gates, absent game, thread affinity, counters and any-thread shutdown passed.");
    return 0;
}
