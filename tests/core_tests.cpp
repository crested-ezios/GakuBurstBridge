#include "../src/GakuBurstBridge.cpp"
#include <cstdlib>
#include <thread>
#define CHECK(expr) do { if (!(expr)) { std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#expr); std::exit(1); } } while (0)
namespace gbb {
struct TestStream {
    uint32_t version{2}, padding{}; uintptr_t pointers[6]{1,1,1,1,0,1};
    GbbQuaternion q[2]{{0,0,0,1},{0,0,0,1}};
    GbbVector3 p[2]{}; int rotations{}, positions{}; bool failResolve{};
};
static float Delta(void*) { return 1.0f / 120.0f; }
static void Resolve(Handle* h, void* raw) { h->skeleton = static_cast<TestStream*>(raw)->failResolve && h->index == 1 ? -1 : h->index; }
static void Read(Handle* h, void* raw, GbbQuaternion* out) { *out = static_cast<TestStream*>(raw)->q[h->index]; }
static void Write(Handle* h, void* raw, GbbQuaternion* q) { auto* s=static_cast<TestStream*>(raw); s->q[h->index]=*q; ++s->rotations; }
static void Position(Handle* h, void* raw, GbbVector3* p) { auto* s=static_cast<TestStream*>(raw); s->p[h->index]=*p; ++s->positions; }
static void LogCount(void* raw, int32_t, const char*) { ++*static_cast<int*>(raw); }
static void SolverCount(void*, void* raw) { ++*static_cast<int*>(raw); }
static Bridge* cancelDuringResolve{};
static void ResolveAndCancel(Handle* h, void* raw) {
    if (auto* bridge=cancelDuringResolve) {
        cancelDuringResolve=nullptr;
        std::thread stop([bridge]{ CHECK(bridge->Shutdown()==GBB_OK); }); stop.join();
    }
    Resolve(h,raw);
}
struct TestMetadataField { const char* name; int offset; uint32_t flags{}; };
static TestMetadataField metadataFields[65];
static int metadataCount;
static void* MetadataDomain() { return reinterpret_cast<void*>(1); }
static void* MetadataAssembly(void*,const char*) { return reinterpret_cast<void*>(1); }
static void* MetadataImage(void*) { return reinterpret_cast<void*>(1); }
static void* MetadataClass(void*,const char*,const char*) { return reinterpret_cast<void*>(1); }
static int MetadataSize(void*,uint32_t* alignment) { *alignment=4; return 12; }
static void* MetadataFields(void*,void** iterator) {
    auto index=reinterpret_cast<uintptr_t>(*iterator);
    if (index>=static_cast<uintptr_t>(metadataCount)) return nullptr;
    *iterator=reinterpret_cast<void*>(index+1); return &metadataFields[index];
}
static const char* MetadataName(void* raw) { return static_cast<TestMetadataField*>(raw)->name; }
static int MetadataOffset(void* raw) { return static_cast<TestMetadataField*>(raw)->offset; }
static uint32_t MetadataFlags(void* raw) { return static_cast<TestMetadataField*>(raw)->flags; }
static void* MetadataType(void*) { return reinterpret_cast<void*>(1); }
static int MetadataTypeCode(void*) { return 8; }
struct BridgeTestAccess {
    static void Run() {
        Bridge b; b.Clock(); b.originalDelta_ = &Delta; b.resolve_ = &Resolve;
        b.readRotation_ = &Read; b.writeRotation_ = &Write; b.writePosition_ = &Position;
        Snapshot s; s.count=2; s.generation=b.generation_.load(); s.inject=true; s.ticks=Ticks();
        s.poses[0].handle.index=0; s.poses[0].hid=0; s.poses[0].rotation={0,0,0,1}; s.poses[0].position={1,2,3};
        s.poses[1].handle.index=1; s.poses[1].hid=10; s.poses[1].rotation={0,1,0,0};
        TestStream stream; Diagnostic d;
        CHECK(b.NativePass(s,&stream,d) && d.writes==2 && d.injected && stream.rotations==2 && stream.positions==1);
        CHECK(d.before.w==1 && d.after.y==1 && stream.p[0].x==1);
        stream={}; stream.failResolve=true; d={};
        CHECK(!b.NativePass(s,&stream,d) && stream.rotations==0 && stream.positions==0);
        stream={}; stream.pointers[5]=0; d={}; CHECK(!b.NativePass(s,&stream,d) && stream.rotations==0);
        stream={}; b.fault_.store(true); d={}; CHECK(b.NativePass(s,&stream,d) && !d.injected && stream.rotations==0);
        b.fault_.store(false); ++s.generation; CHECK(!b.NativePass(s,&stream,d) && stream.rotations==0); --s.generation;
        alignas(16) unsigned char job[1024]{}; uintptr_t hair=123,arm=456;
        std::memcpy(job+profile::Hair,&hair,8); std::memcpy(job+profile::Arm,&arm,8);
        s.hair=hair; s.arm=arm; s.ticks=Ticks(); b.current_=s; stream={};
        CHECK(b.BeforeSwing(job+profile::Swing,&stream) && b.matches_.load()==1 && b.nativeTimer_.calls.load()==1);
        CHECK(stream.rotations==2 && b.diagnostic_.writes==2);
        b.current_.ticks=Ticks()-b.frequency_.load(); stream={};
        CHECK(b.BeforeSwing(job+profile::Swing,&stream) && stream.rotations==0 && b.expired_.load()==1);
        b.current_.arm=999; CHECK(!b.BeforeSwing(job+profile::Swing,&stream) && b.identitySkips_.load()==1);
        b.current_=s; b.current_.ticks=Ticks(); stream={}; stream.failResolve=true;
        CHECK(b.BeforeSwing(job+profile::Swing,&stream) && b.fault_.load() && b.failures_.load()==1);
        const auto generation=b.generation_.load(); b.Cancel(false);
        CHECK(b.current_.count==0 && b.generation_.load()>generation && b.fault_.load());
        b.Cancel(true); CHECK(!b.fault_.load());
        b.burst_=reinterpret_cast<HMODULE>(job); b.burstSize_=sizeof(job);
        const auto caller=reinterpret_cast<uintptr_t>(job+8);
        b.CaptureTrace(caller,.01f); b.CaptureTrace(caller,.02f); b.CaptureTrace(caller,.005f);
        CHECK(b.traces_[0].calls==3 && b.traces_[0].caller==8 && b.traces_[0].first==.01f &&
              b.traces_[0].minimum==.005f && b.traces_[0].maximum==.02f);
        for(int i=1;i<33;++i) b.CaptureTrace(caller+i,.01f);
        CHECK(b.traces_[31].calls==1 && b.droppedTraces_.load()==1);
        b.CaptureTrace(reinterpret_cast<uintptr_t>(job)-1,.01f);
        CHECK(b.droppedTraces_.load()==1); // Other modules do not consume slots.
        b.Cancel(true); CHECK(b.traces_[0].calls==3); // Evidence survives Clear.
        b.mainThread_.store(GetCurrentThreadId());
        int logged{};
        GbbConfig config{}; config.size=sizeof(config); config.version=GBB_API_VERSION;
        config.log=&LogCount; config.log_context=&logged;
        CHECK(b.Configure(&config)==GBB_OK);
        b.State(GBB_STATE_READY,GBB_OK,"Test callback before shutdown"); CHECK(logged==1);
        s.generation=b.generation_.load(); s.ticks=Ticks(); b.current_=s;
        const auto beforeShutdown=b.generation_.load(), matches=b.matches_.load(), failed=b.failures_.load();
        int shutdownResult{};
        std::thread shutdown([&]{ shutdownResult=b.Shutdown(); }); shutdown.join();
        CHECK(shutdownResult==GBB_OK && b.stopped_.load() && !b.observing_.load());
        CHECK(b.current_.count==0 && b.generation_.load()>beforeShutdown && b.log_==nullptr && b.logContext_==nullptr);
        CHECK(logged==1 && b.state_==GBB_STATE_SHUTDOWN && b.result_==GBB_E_SHUTDOWN);
        b.State(GBB_STATE_INJECTING,GBB_OK,"Stale main-thread result");
        CHECK(logged==1 && b.state_==GBB_STATE_SHUTDOWN);
        stream={}; d={}; CHECK(!b.NativePass(s,&stream,d) && stream.rotations==0);
        CHECK(!b.BeforeSwing(job+profile::Swing,&stream) && stream.rotations==0);
        CHECK(b.matches_.load()==matches && b.failures_.load()==failed);
        b.originalSwing_=&SolverCount;
        int solverCalls{}; const auto swingCalls=b.swingCalls_.load(), probeCalls=b.probeCalls_.load();
        Bridge::SwingDetour(nullptr,&solverCalls); CHECK(solverCalls==1 && b.swingCalls_.load()==swingCalls);
        CHECK(Bridge::DeltaDetour(nullptr)==1.0f/120.0f && b.probeCalls_.load()==probeCalls);
        const auto stoppedGeneration=b.generation_.load();
        CHECK(b.Shutdown()==GBB_OK && b.generation_.load()==stoppedGeneration);
        CHECK(b.Configure(&config)==GBB_E_SHUTDOWN && b.Clear(0)==GBB_E_SHUTDOWN && b.Publish(nullptr)==GBB_E_SHUTDOWN);
        // Stop while a worker pass holds a copied snapshot. Shutdown must not
        // allow the old pass to publish a failure/fault or start stream writes.
        Bridge racing; racing.Clock(); racing.originalDelta_=&Delta; racing.resolve_=&ResolveAndCancel;
        racing.readRotation_=&Read; racing.writeRotation_=&Write; racing.writePosition_=&Position;
        s.generation=racing.generation_.load(); s.ticks=Ticks(); racing.current_=s;
        cancelDuringResolve=&racing; stream={};
        CHECK(racing.BeforeSwing(job+profile::Swing,&stream));
        CHECK(racing.stopped_.load() && racing.state_==GBB_STATE_SHUTDOWN && stream.rotations==0);
        CHECK(racing.matches_.load()==0 && racing.failures_.load()==0 && !racing.fault_.load() && racing.diagnostic_.sequence==0);
        // Rejected initialization still permits bounded explicit metadata
        // export. Raw boxed offsets remain evidence and the header assumption
        // must not be promoted into an independently observed value.
        racing.metadataLoaded_=true; racing.failedInitialization_=true;
        auto& meta=racing.meta_;
        meta.domainGet=&MetadataDomain; meta.assemblyOpen=&MetadataAssembly; meta.assemblyImage=&MetadataImage;
        meta.classFind=&MetadataClass; meta.valueSize=&MetadataSize; meta.classFields=&MetadataFields;
        meta.fieldName=&MetadataName; meta.fieldOffset=&MetadataOffset; meta.fieldFlags=&MetadataFlags;
        meta.fieldType=&MetadataType; meta.typeCode=&MetadataTypeCode;
        metadataFields[0]={"m_AnimatorBindingsVersion",16};
        metadataFields[1]={"handleIndex",20}; metadataFields[2]={"skeletonIndex",24};
        metadataFields[3]={"Static\\\"\nField",0,0x10}; metadataCount=4;
        std::string abiReport; racing.AppendAbiReport(abiReport);
        CHECK(abiReport.find("\"metadata_report_available\": true")!=std::string::npos);
        CHECK(abiReport.find("\"handle_value_size\":12")!=std::string::npos);
        CHECK(abiReport.find("\"handle_fields_unboxed\":{\"m_AnimatorBindingsVersion\":0,\"handleIndex\":4,\"skeletonIndex\":8}")!=std::string::npos);
        CHECK(abiReport.find("\"raw_boxed_offset\":20")!=std::string::npos);
        CHECK(abiReport.find("Static\\\\\\\"\\u000aField")!=std::string::npos);
        CHECK(abiReport.find("\"fields_truncated\":false")!=std::string::npos);
        CHECK(abiReport.find("\"observed\": {\"pointer_size\":8,\"boxed_header_size\"")==std::string::npos);
        for(int i=4;i<65;++i) metadataFields[i]={"Extra",28}; metadataCount=65;
        abiReport.clear(); racing.AppendAbiReport(abiReport);
        CHECK(abiReport.find("\"fields_truncated\":true")!=std::string::npos);
        CHECK(racing.state_==GBB_STATE_SHUTDOWN && racing.failedInitialization_ && !racing.initialized_);
        selfReset();
    }
    static void selfReset() { Bridge::self_=&instance; }
};
}
static void TestProfile() {
    using namespace gbb;
    auto* bytes=static_cast<unsigned char*>(VirtualAlloc(nullptr,0x200000,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE)); CHECK(bytes);
    auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(bytes); dos->e_magic=IMAGE_DOS_SIGNATURE; dos->e_lfanew=0x80;
    auto* nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(bytes+0x80); nt->Signature=IMAGE_NT_SIGNATURE;
    nt->FileHeader.Machine=IMAGE_FILE_MACHINE_AMD64; nt->OptionalHeader.Magic=IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    nt->OptionalHeader.SizeOfImage=0x200000;
    bytes[0x160000]=1; // valid minimal UNWIND_INFO
    RUNTIME_FUNCTION functions[2]{};
    for(int i=0;i<2;++i) {
        const auto& v=profile::Variants[i]; functions[i]={v.begin,v.end,0x160000};
        std::memcpy(bytes+v.begin,profile::Entry,sizeof(profile::Entry));
        bytes[v.returned-6]=0xff; bytes[v.returned-5]=0x15;
        int32_t displacement=profile::DeltaSlot-v.returned; std::memcpy(bytes+v.returned-4,&displacement,4);
    }
    void* target=reinterpret_cast<void*>(&gbb::Delta); std::memcpy(bytes+profile::DeltaSlot,&target,8);
    CHECK(RtlAddFunctionTable(functions,2,reinterpret_cast<DWORD64>(bytes)));
    for(const auto& v:profile::Variants) {
        auto caller=reinterpret_cast<uintptr_t>(bytes+v.returned);
        CHECK(profile::VerifyCaller(caller,reinterpret_cast<HMODULE>(bytes),target)==reinterpret_cast<uintptr_t>(bytes+v.begin));
        bytes[v.begin]=0x90; CHECK(!profile::VerifyCaller(caller,reinterpret_cast<HMODULE>(bytes),target)); bytes[v.begin]=profile::Entry[0];
        CHECK(!profile::VerifyCaller(caller+1,reinterpret_cast<HMODULE>(bytes),target));
        CHECK(!profile::VerifyCaller(caller,reinterpret_cast<HMODULE>(bytes),reinterpret_cast<void*>(1)));
    }
    nt->FileHeader.Machine=IMAGE_FILE_MACHINE_I386;
    CHECK(!profile::VerifyCaller(reinterpret_cast<uintptr_t>(bytes+profile::Variants[0].returned),reinterpret_cast<HMODULE>(bytes),target));
    CHECK(RtlDeleteFunctionTable(functions)); CHECK(VirtualFree(bytes,0,MEM_RELEASE));
}
int main() {
    gbb::BridgeTestAccess::Run(); TestProfile();
    std::puts("Core tests: stream/write gates, bounded traces, SSE/AVX PE/unwind guards and forwarding-only any-thread shutdown passed.");
    return 0;
}
