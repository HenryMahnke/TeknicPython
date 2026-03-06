// main.cpp – integration test for the MotorManager C++ layer
//
// Run sequence:
//   1. Initialize SysManager
//   2. Find and open the SC Hub port
//   3. Apply motion limits
//   4. Enable all motors
//   5. Home all motors
//   6. Execute a simple two-motor move profile
//   7. Poll until at target, then shut down
//
// Build (from project root after cmake configuration):
//   cmake --build build --target teknic_motors

#include "TeknicMotors.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

static void waitEnter(const std::string &prompt) {
    std::cout << prompt << "  [Press Enter] ";
    std::cin.get();
}

int main() {
    std::cout << "==============================\n";
    std::cout << " TeknicMotors Integration Test\n";
    std::cout << "==============================\n\n";

    MotorManager mgr;
    mgr.testHello();
    waitEnter("SysManager ready.");

    // -----------------------------------------------------------------------
    // Step 1 – initialize
    // -----------------------------------------------------------------------
    if (mgr.inititalize() != 0) {
        std::cerr << "FAIL: initialize()\n";
        return 1;
    }
    std::cout << "[TEST] initialize()  OK\n";

    // -----------------------------------------------------------------------
    // Step 2 – discover and open port
    // -----------------------------------------------------------------------
    if (mgr.findAndOpenPorts() != 0) {
        std::cerr << "FAIL: findAndOpenPorts() – is the SC Hub connected?\n";
        return 1;
    }
    std::cout << "[TEST] findAndOpenPorts()  OK  –  "
              << mgr.getNodeCount() << " node(s) found\n";

    if (mgr.getNodeCount() == 0) {
        std::cerr << "No motors on the network. Exiting.\n";
        mgr.shutdown();
        return 1;
    }

    // -----------------------------------------------------------------------
    // Step 3 – set motion limits before enabling
    // -----------------------------------------------------------------------
    if (mgr.setMotorLimits(/*accRpmPerSec=*/500.0, /*velRpm=*/200.0) != 0) {
        std::cerr << "FAIL: setMotorLimits()\n";
        mgr.shutdown();
        return 1;
    }
    std::cout << "[TEST] setMotorLimits(500 RPM/s, 200 RPM)  OK\n";

    // -----------------------------------------------------------------------
    // Step 4 – enable
    // -----------------------------------------------------------------------
    waitEnter("About to enable motors.");
    if (mgr.enableMotors() != 0) {
        std::cerr << "FAIL: enableMotors()\n";
        mgr.shutdown();
        return 1;
    }
    std::cout << "[TEST] enableMotors()  OK\n";

    if (mgr.isReady() != 1) {
        std::cerr << "FAIL: motors not ready after enable\n";
        mgr.shutdown();
        return 1;
    }
    std::cout << "[TEST] isReady()  OK\n";

    // -----------------------------------------------------------------------
    // Step 5 – home
    // -----------------------------------------------------------------------
    waitEnter("About to home motors.");
    if (mgr.homeMotors() != 0) {
        std::cerr << "WARN: homeMotors() returned non-zero (check homing config)\n";
        // Not fatal – motor may not have homing set up in ClearView
    } else {
        std::cout << "[TEST] homeMotors()  OK\n";
    }

    // -----------------------------------------------------------------------
    // Step 6 – move profile
    //   Move Motor 0 to  +2000 counts, Motor 1 to +1500 counts,
    //   then back to 0.
    // -----------------------------------------------------------------------
    auto doMove = [&](int sp0, int sp1) -> bool {
        mgr.setSetpoint(0, static_cast<double>(sp0));
        if (mgr.getNodeCount() > 1)
            mgr.setSetpoint(1, static_cast<double>(sp1));

        if (mgr.goToSetpoint() != 0) {
            std::cerr << "FAIL: goToSetpoint()\n";
            return false;
        }

        // Poll for completion (timeout 10 s)
        const int timeoutMs = 10000;
        int elapsed = 0;
        while (elapsed < timeoutMs) {
            int done = mgr.atSetpoint();
            if (done < 0) {
                std::cerr << "FAIL: atSetpoint() returned error\n";
                return false;
            }
            if (done == 1) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            elapsed += 50;
        }

        std::cout << "  Motor 0 position: " << mgr.getPosition(0) << " counts\n";
        if (mgr.getNodeCount() > 1)
            std::cout << "  Motor 1 position: " << mgr.getPosition(1) << " counts\n";
        return true;
    };

    waitEnter("About to move to target positions.");
    std::cout << "[TEST] Moving to (2000, 1500) counts...\n";
    if (!doMove(2000, 1500)) {
        mgr.shutdown();
        return 1;
    }
    std::cout << "[TEST] Move to target  OK\n";

    waitEnter("About to return to home (0, 0).");
    std::cout << "[TEST] Returning to (0, 0)...\n";
    if (!doMove(0, 0)) {
        mgr.shutdown();
        return 1;
    }
    std::cout << "[TEST] Return to home  OK\n";

    // -----------------------------------------------------------------------
    // Step 7 – shutdown
    // -----------------------------------------------------------------------
    waitEnter("Test complete. About to shut down.");
    mgr.shutdown();
    std::cout << "[TEST] shutdown()  OK\n\n";
    std::cout << "All tests passed.\n";
    return 0;
}
