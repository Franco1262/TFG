import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess


def function_body(source, name):
    match = re.search(r"static void " + name + r"\(\)\s*\{(.*?)^\}", source, re.M | re.S)
    if not match:
        raise RuntimeError(f"Cannot find {name} in main.cpp")
    return match.group(1).replace("Lora::", "radio.").replace("SD::", "sd.")


def main():
    parser = argparse.ArgumentParser(description="Compile-time batch-flow checks using real firmware functions and simulated hardware.")
    parser.add_argument("--compiler")
    arguments = parser.parse_args()
    project = Path(__file__).resolve().parents[1]
    compiler = arguments.compiler or os.environ.get("CXX") or shutil.which("g++")
    if not compiler:
        compiler = str(Path.home() / ".platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-g++.exe")
    source = (project / "src/main.cpp").read_text(encoding="utf-8")
    sd_source = (project / "src/SD.cpp").read_text(encoding="utf-8")
    sd_match = re.search(r"bool SD::logBatch\(.*?\n\}", sd_source, re.S)
    if not sd_match:
        raise RuntimeError("Cannot find SD::logBatch in SD.cpp")
    sd_body = sd_match.group()
    write_check = re.search(
        r"file\.write\(reinterpret_cast<const uint8_t\*>\(json_buffer\), line_length\) != line_length",
        sd_body,
    )
    written_increment = sd_body.find("written++")
    if not write_check or written_increment < write_check.end():
        raise RuntimeError("SD::logBatch must reject a partial line before counting it as written")
    if "json_length < 0" not in sd_body or not re.search(
        r"json_length\)\s*>=\s*sizeof\(json_buffer\)\s*-\s*1", sd_body
    ):
        raise RuntimeError("SD::logBatch must reject formatting errors and truncated JSON lines")
    types = (project / "include/types.h").read_text(encoding="utf-8")
    types = re.sub(r"^\s*#.*$", "", types, flags=re.M)
    buffer = (project / "include/CircularBuffer.h").read_text(encoding="utf-8")
    buffer = buffer.replace("#pragma once", "")
    buffer = re.sub(
        r"^(\s*)(void clear|bool full|bool empty|int size|bool push|bool pop|bool discard|void discard|Iterator& operator\+\+|bool operator!=|const T& operator\*|Iterator begin|Iterator end)",
        r"\1constexpr \2", buffer, flags=re.M,
    )
    declarations = r'''
#include <cstdint>
#include <cstddef>
#include "config.h"
#define DEBUG_PRINTLN(...) ((void)0)
#define DEBUG_PRINTF(...) ((void)0)
constexpr int16_t RADIOLIB_ERR_NONE = 0;
constexpr int16_t RADIOLIB_ERR_UPLINK_UNAVAILABLE = -1108;
'''
    hardware = r'''
struct TestRadio {
    bool starts = true;
    bool session_ready = true;
    uint8_t capacity = 4;
    int16_t send_status = 0;
    int16_t session_status = -1;
    int starts_count = 0;
    int shutdowns = 0;
    int sends = 0;
    int saved_sessions = 0;
    uint8_t requested = 0;

    constexpr bool turnOn() {
        starts_count++;
        if (!starts) shutdowns++;
        return starts;
    }
    constexpr void turnOff() { shutdowns++; }
    constexpr bool ensureSessionReady() { return session_ready; }
    constexpr int16_t getSessionStatus() { return session_status; }
    constexpr uint8_t getMaxItemsPerPacket() { return capacity; }
    constexpr void saveLoRaSessionToRTC() { saved_sessions++; }
    constexpr uint8_t sendBatch(const CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE>&, size_t count, int16_t* result) {
        sends++;
        requested = count;
        *result = send_status;
        return send_status < 0 ? 0 : count;
    }
};

struct TestSD {
    bool mounts = true;
    bool writes = true;
    int power_ons = 0;
    int shutdowns = 0;
    int log_attempts = 0;
    int logged = 0;
    uint32_t ids[1024] = {};

    constexpr void powerOn() { power_ons++; }
    constexpr bool mount() { return mounts; }
    constexpr bool logBatch(const CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE>& samples, uint8_t count) {
        log_attempts++;
        if (!writes) return false;
        uint8_t copied = 0;
        for (const auto& sample : samples) {
            if (copied >= count) break;
            ids[logged++] = sample.timestamp;
            copied++;
        }
        return copied == count;
    }
    constexpr void shutdown() { shutdowns++; }
};
'''
    application = r'''
struct TestApp {
    CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE> rtc_buffer = {};
    PendingBatch pending_batch = {};
    uint32_t rtc_buffer_front_id = 0;
    uint32_t rtc_buffer_magic = 0;
    TestRadio radio = {};
    TestSD sd = {};
    constexpr bool append(uint32_t id) {
        Data sample = {};
        sample.timestamp = id;
        return rtc_buffer.push(sample);
    }
    constexpr void fill(uint32_t first, uint32_t count) {
        for (uint32_t offset = 0; offset < count; offset++) append(first + offset);
    }
    constexpr uint32_t firstQueued() const {
        return rtc_buffer.empty() ? 0 : (*rtc_buffer.begin()).timestamp;
    }
'''
    magic = re.search(r"static constexpr uint32_t RTC_BUFFER_MAGIC = ([^;]+);", source)
    if not magic:
        raise RuntimeError("Cannot find RTC buffer signature")
    application += f"static constexpr uint32_t RTC_BUFFER_MAGIC = {magic.group(1)};\n"
    for name in ("initRtcBuffer", "finalizeBatch", "processBatch"):
        application += f"constexpr void {name}() {{{function_body(source, name)}}}\n"
    application += "};\n"
    tests = r'''
constexpr bool belowThreshold() {
    TestApp app = {};
    app.fill(1, 3);
    app.processBatch();
    return app.radio.starts_count == 0 && !app.pending_batch.active() && app.sd.logged == 0;
}
constexpr bool waitForFullPacket() {
    TestApp app = {};
    app.fill(1, 4);
    app.radio.capacity = 8;
    app.processBatch();
    return app.radio.sends == 0 && app.radio.shutdowns == 1 && !app.pending_batch.active() && app.rtc_buffer.size() == 4;
}
constexpr bool firstSuccess() {
    TestApp app = {};
    app.fill(1, 4);
    app.processBatch();
    return app.sd.logged == 4 && app.sd.ids[0] == 1 && app.sd.ids[3] == 4 && app.rtc_buffer.empty() && app.rtc_buffer_front_id == 4 && !app.pending_batch.active() && app.radio.saved_sessions == 1;
}
constexpr bool retryKeepsOriginalLot() {
    TestApp app = {};
    app.fill(1, 4);
    app.radio.send_status = -1;
    app.processBatch();
    if (app.pending_batch.attempts != 1 || app.sd.log_attempts != 0 || app.rtc_buffer.size() != 4) return false;
    app.fill(5, 2);
    app.radio.send_status = 0;
    app.radio.capacity = 12;
    app.processBatch();
    return app.radio.requested == 4 && app.sd.logged == 4 && app.firstQueued() == 5 && app.rtc_buffer.size() == 2 && !app.pending_batch.active();
}
constexpr bool reducedRetry(int16_t result) {
    TestApp app = {};
    app.fill(1, 8);
    app.radio.capacity = 8;
    app.radio.send_status = -1;
    app.processBatch();
    app.fill(9, 2);
    app.radio.capacity = 5;
    app.radio.send_status = result;
    app.processBatch();
    return app.radio.requested == 5 && app.sd.log_attempts == 1 && app.sd.logged == 5 && app.sd.ids[4] == 5 && app.firstQueued() == 6 && app.rtc_buffer.size() == 5 && app.rtc_buffer_front_id == 5 && !app.pending_batch.active();
}
constexpr bool zeroCapacityFinalizesOriginalLot() {
    TestApp app = {};
    app.fill(1, 8);
    app.radio.capacity = 8;
    app.radio.send_status = -1;
    app.processBatch();
    app.fill(9, 2);
    app.radio.capacity = 0;
    app.processBatch();
    return app.radio.sends == 1 && app.sd.logged == 8 && app.rtc_buffer.size() == 2 && app.firstQueued() == 9;
}
constexpr bool radioFailuresUseFourSamples() {
    TestApp app = {};
    app.fill(1, 6);
    app.radio.starts = false;
    app.processBatch();
    if (app.pending_batch.count != 4 || app.pending_batch.attempts != 1 || app.sd.log_attempts != 0) return false;
    app.append(7);
    app.processBatch();
    return app.sd.logged == 4 && app.rtc_buffer.size() == 3 && app.firstQueued() == 5 && app.radio.sends == 0;
}
constexpr bool joinFailureThenSuccess() {
    TestApp app = {};
    app.fill(1, 6);
    app.radio.session_ready = false;
    app.processBatch();
    if (app.pending_batch.attempts != 1 || app.pending_batch.count != 4) return false;
    app.append(7);
    app.radio.session_ready = true;
    app.radio.capacity = 8;
    app.processBatch();
    return app.radio.requested == 4 && app.sd.logged == 4 && app.rtc_buffer.size() == 3 && app.firstQueued() == 5;
}
constexpr bool joinFailureTwice() {
    TestApp app = {};
    app.fill(1, 6);
    app.radio.session_ready = false;
    app.processBatch();
    app.processBatch();
    return !app.pending_batch.active() && app.sd.logged == 4 && app.rtc_buffer.size() == 2 && app.radio.sends == 0;
}
constexpr bool sessionDeferralCostsNoAttempt() {
    TestApp app = {};
    app.fill(1, 4);
    app.radio.session_ready = false;
    app.radio.session_status = RADIOLIB_ERR_UPLINK_UNAVAILABLE;
    app.processBatch();
    return app.pending_batch.attempts == 0 && app.rtc_buffer.size() == 4 && app.sd.log_attempts == 0 && app.radio.shutdowns == 1;
}
constexpr bool sendDeferralPreservesRetry() {
    TestApp app = {};
    app.fill(1, 8);
    app.radio.capacity = 8;
    app.radio.send_status = -1;
    app.processBatch();
    app.append(9);
    app.radio.capacity = 5;
    app.radio.send_status = RADIOLIB_ERR_UPLINK_UNAVAILABLE;
    app.processBatch();
    if (app.pending_batch.attempts != 1 || app.pending_batch.count != 8 || app.rtc_buffer.size() != 9 || app.sd.log_attempts != 0) return false;
    app.radio.send_status = 0;
    app.processBatch();
    return app.sd.logged == 5 && app.firstQueued() == 6 && app.rtc_buffer.size() == 4;
}
constexpr bool sdMountFailureStillFinalizes() {
    TestApp app = {};
    app.fill(1, 4);
    app.sd.mounts = false;
    app.processBatch();
    return app.rtc_buffer.empty() && !app.pending_batch.active() && app.sd.log_attempts == 0 && app.sd.shutdowns == 1;
}
constexpr bool sdWriteFailureDoesNotRetryLog() {
    TestApp app = {};
    app.fill(1, 4);
    app.radio.send_status = -1;
    app.sd.writes = false;
    app.processBatch();
    app.processBatch();
    app.processBatch();
    return app.rtc_buffer.empty() && !app.pending_batch.active() && app.sd.log_attempts == 1 && app.sd.logged == 0;
}
constexpr bool fullBufferDoesNotOverwrite() {
    TestApp app = {};
    app.fill(1, MAX_ABSOLUTE_BATCH_SIZE);
    return !app.append(9999) && app.rtc_buffer.size() == MAX_ABSOLUTE_BATCH_SIZE && app.firstQueued() == 1;
}
constexpr bool wraparoundKeepsOrdering() {
    TestApp app = {};
    for (uint32_t cycle = 0; cycle < 70; cycle++) {
        app.fill(cycle * 4 + 1, 4);
        app.processBatch();
    }
    return app.rtc_buffer.empty() && app.rtc_buffer_front_id == 280 && app.sd.logged == 280 && app.sd.ids[255] == 256 && app.sd.ids[279] == 280;
}
constexpr bool retainedStateSurvivesInitialization() {
    TestApp app = {};
    app.initRtcBuffer();
    app.fill(1, 4);
    app.radio.send_status = -1;
    app.processBatch();
    app.initRtcBuffer();
    app.append(5);
    app.radio.send_status = 0;
    app.processBatch();
    return app.sd.logged == 4 && app.firstQueued() == 5 && app.rtc_buffer.size() == 1;
}
constexpr bool invalidReferenceDoesNotDiscard() {
    TestApp app = {};
    app.fill(1, 4);
    app.pending_batch = {99, 4, 1};
    app.processBatch();
    return app.rtc_buffer.size() == 4 && !app.pending_batch.active() && app.radio.starts_count == 0 && app.sd.log_attempts == 0;
}
constexpr bool remainderGetsNewBudget() {
    TestApp app = {};
    app.fill(1, 8);
    app.radio.capacity = 8;
    app.radio.send_status = -1;
    app.processBatch();
    app.radio.capacity = 5;
    app.processBatch();
    app.append(9);
    app.radio.capacity = 4;
    app.processBatch();
    return app.pending_batch.first_sample == 5 && app.pending_batch.count == 4 && app.pending_batch.attempts == 1 && app.firstQueued() == 6 && app.sd.logged == 5;
}
static_assert(sizeof(Data) == 19, "Wire format must remain 19 bytes");
static_assert(belowThreshold(), "Below threshold");
static_assert(waitForFullPacket(), "Wait for a complete first packet");
static_assert(firstSuccess(), "First-attempt success");
static_assert(retryKeepsOriginalLot(), "New samples must not join the retry");
static_assert(reducedRetry(0), "Successful reduced second attempt");
static_assert(reducedRetry(-1), "Failed reduced second attempt");
static_assert(zeroCapacityFinalizesOriginalLot(), "Zero-capacity second attempt");
static_assert(radioFailuresUseFourSamples(), "Radio-init failures");
static_assert(joinFailureThenSuccess(), "Join failure then successful retry");
static_assert(joinFailureTwice(), "Two join failures");
static_assert(sessionDeferralCostsNoAttempt(), "Join duty-cycle deferral");
static_assert(sendDeferralPreservesRetry(), "Send duty-cycle deferral");
static_assert(sdMountFailureStillFinalizes(), "SD mount failure");
static_assert(sdWriteFailureDoesNotRetryLog(), "SD write failure");
static_assert(fullBufferDoesNotOverwrite(), "Full buffer");
static_assert(wraparoundKeepsOrdering(), "Circular-buffer wraparound");
static_assert(retainedStateSurvivesInitialization(), "Retained pending state");
static_assert(invalidReferenceDoesNotDiscard(), "Invalid batch reference");
static_assert(remainderGetsNewBudget(), "Remainder starts a new lot");
'''
    program = declarations + types + buffer + hardware + application + tests
    completed = subprocess.run(
        [compiler, "-std=gnu++14", "-x", "c++", "-fsyntax-only", "-I", str(project / "include"), "-"],
        input=program, text=True, capture_output=True,
    )
    if completed.returncode:
        raise SystemExit(completed.stdout + completed.stderr)
    print(f"PASS: {tests.count('static_assert(')} compile-time assertions on real batch-flow code with simulated hardware")
    print("PASS: SD::logBatch rejects partial writes and truncated JSON before counting a line")
    print("The buffer and flow are made constexpr only in this test copy; production code is unchanged.")
    print("The SD assertion checks source ordering only; it does not simulate card I/O or power loss.")
    print("This does not validate radio hardware, SD writes or physical RTC retention.")


if __name__ == "__main__":
    main()