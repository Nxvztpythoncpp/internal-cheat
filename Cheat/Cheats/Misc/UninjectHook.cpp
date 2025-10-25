#include "UninjectHook.h"
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "../../Dependencies/Kiero/kiero.h"
#include "../../Gui/Imgui/backends/imgui_impl_dx9.h"
#include "../../Gui/Imgui/imgui.h"

using namespace std::chrono_literals;

// Se tens um Menu global/namespace, inclui-o aqui. Exemplo mínimo abaixo:
namespace Menu {
    inline bool menuOpen = true;
}

namespace Unhook {

    static std::atomic<bool> cleaning{ false };
    static std::atomic<bool> unloadRequested{ false };
    static std::vector<std::thread> workerThreads;
    static std::mutex workerMutex;
    static std::condition_variable workerCv;

    static void TryUnbindHooksSafe() {
        std::cout << "[Unhook] TryUnbindHooksSafe() called (placeholder).\n";
        UnbindHooks();
    }

    static void SignalWorkersToStop() {
        std::cout << "[Unhook] SignalWorkersToStop() called.\n";
        workerCv.notify_all();
    }

    static void JoinWorkerThreads() {
        std::lock_guard<std::mutex> lk(workerMutex);
        std::cout << "[Unhook] Joining " << workerThreads.size() << " worker threads...\n";
        for (auto& t : workerThreads) {
            if (t.joinable()) {
                try { t.join(); }
                catch (const std::exception& ex) {
                    std::cerr << "[Unhook] Exception while joining thread: " << ex.what() << '\n';
                }
                catch (...) {
                    std::cerr << "[Unhook] Unknown exception while joining thread.\n";
                }
            }
        }
        workerThreads.clear();
    }

    static void UndoPatchedState() {
        std::cout << "[Unhook] UndoPatchedState() called.\n";
    }

    static void ReleaseResources() {
        std::cout << "[Unhook] ReleaseResources() called.\n";
        GuiUnhook();
    }

    static void CleanupProcedure() {
        if (cleaning.exchange(true)) {
            std::cout << "[Unhook] Cleanup already in progress; returning.\n";
            return;
        }

        std::cout << "[Unhook] Cleanup thread started.\n";

        Menu::menuOpen = false;
        TryUnbindHooksSafe();
        SignalWorkersToStop();
        std::this_thread::sleep_for(10ms);

        JoinWorkerThreads();
        UndoPatchedState();
        ReleaseResources();

        cleaning = false;
        std::cout << "[Unhook] Cleanup finished.\n";
    }

    void StartSafeCleanupThread() {
        if (cleaning.load()) return;
        std::thread([]() { CleanupProcedure(); }).detach();
    }

    void RequestUnload() {
        unloadRequested = true;
        Menu::menuOpen = false;
        StartSafeCleanupThread();
    }

    bool IsCleaning() {
        return cleaning.load();
    }

    void RegisterWorkerThread(std::thread t) {
        std::lock_guard<std::mutex> lk(workerMutex);
        workerThreads.emplace_back(std::move(t));
    }

    bool ShouldWorkersStop() {
        return unloadRequested.load();
    }


    void UnbindHooks() {
        kiero::unbind(42);
        //kiero::shutdown(); if we cant fix we have a ragequit here HAHAHAHAHWHAHAHAH
        std::cout << "[Unhook] UnbindHooks() placeholder called. No action by default.\n";
    }

    void GuiUnhook() {
        //menu still injected I forget the shit for close the menu before that 
        //ImGui::DestroyContext(); //CRASHING WTF
    }

}
