#include "TeknicMotors.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>


using namespace sFnd;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Millisecond sleep compatible with the Linux sFoundation target
static void sleepMs(unsigned int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Poll a condition functor until it is true or a timeout expires.
// Returns true if the condition was met, false on timeout.
template <typename Pred>
static bool waitFor(Pred pred, unsigned int timeoutMs,
                    unsigned int pollMs = 10) {
  unsigned int elapsed = 0;
  while (elapsed < timeoutMs) {
    if (pred())
      return true;
    sleepMs(pollMs);
    elapsed += pollMs;
  }
  return false;
}

// ---------------------------------------------------------------------------
// Private helper: get a node by index, throws if out of range
// ---------------------------------------------------------------------------
INode *MotorManager::getNode(int motorIdx) {
  if (!myPort) {
    throw std::runtime_error("Port not open. Call findAndOpenPorts() first.");
  }
  int count = static_cast<int>(myPort->NodeCount());
  if (motorIdx < 0 || motorIdx >= count) {
    throw std::out_of_range(
        "motorIdx " + std::to_string(motorIdx) +
        " is out of range (nodeCount=" + std::to_string(count) + ")");
  }
  return &myPort->Nodes(motorIdx);
}

// ---------------------------------------------------------------------------
// initialize
//   Creates the SysManager instance. Safe to call multiple times; subsequent
//   calls are a no-op if the manager already exists.
// ---------------------------------------------------------------------------
int MotorManager::inititalize() {
  try {
    if (!myMgr) {
      myMgr = SysManager::Instance();
      std::cout << "[MotorManager] SysManager instance created.\n";
    }
  } catch (mnErr &e) {
    std::cerr << "[MotorManager] initialize error: " << e.ErrorMsg << "\n";
    return -1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
// findAndOpenPorts
//   Discovers SC Hub COM ports, opens the first one found, caches Motor1 /
//   Motor2 pointers if at least two nodes are present.
// ---------------------------------------------------------------------------
int MotorManager::findAndOpenPorts() {
  if (!myMgr) {
    std::cerr << "[MotorManager] Call initialize() first.\n";
    return -1;
  }
  try {
    std::vector<std::string> comHubPorts;
    SysManager::FindComHubPorts(comHubPorts);

    if (comHubPorts.empty()) {
      std::cerr << "[MotorManager] No SC Hub ports found.\n";
      return -1;
    }

    std::cout << "[MotorManager] Found " << comHubPorts.size()
              << " SC Hub port(s).\n";

    // Register and open only the first hub (port 0)
    size_t portCount =
        std::min(comHubPorts.size(), static_cast<size_t>(NET_CONTROLLER_MAX));
    for (size_t i = 0; i < portCount; ++i) {
      myMgr->ComHubPort(static_cast<int>(i), comHubPorts[i].c_str());
    }

    myMgr->PortsOpen(static_cast<int>(portCount));

    myPort = &myMgr->Ports(0);
    std::cout << "[MotorManager] Port 0 opened. State=" << myPort->OpenState()
              << "  Nodes=" << myPort->NodeCount() << "\n";

    // Cache convenience pointers
    int n = static_cast<int>(myPort->NodeCount());
    Motor1 = (n > 0) ? &myPort->Nodes(0) : nullptr;
    Motor2 = (n > 1) ? &myPort->Nodes(1) : nullptr;

  } catch (mnErr &e) {
    std::cerr << "[MotorManager] findAndOpenPorts error: " << e.ErrorMsg
              << "\n";
    return -1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
// enableMotors
//   Enables every node on the open port and waits for them to become ready.
// ---------------------------------------------------------------------------
int MotorManager::enableMotors() {
  if (!myPort) {
    std::cerr << "[MotorManager] No port open.\n";
    return -1;
  }
  try {
    for (size_t i = 0; i < myPort->NodeCount(); ++i) {
      INode &node = myPort->Nodes(i);
      node.EnableReq(true);
      std::cout << "[MotorManager] Node " << i << " enable requested.\n";
    }

    // Wait up to 3 seconds for all nodes to enable
    bool allReady = waitFor(
        [this]() {
          for (size_t i = 0; i < myPort->NodeCount(); ++i) {
            if (!myPort->Nodes(i).Motion.IsReady())
              return false;
          }
          return true;
        },
        3000);

    if (!allReady) {
      std::cerr << "[MotorManager] Timeout waiting for motors to enable.\n";
      return -1;
    }
    std::cout << "[MotorManager] All motors enabled.\n";

  } catch (mnErr &e) {
    std::cerr << "[MotorManager] enableMotors error: " << e.ErrorMsg << "\n";
    return -1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
// disableMotors
// ---------------------------------------------------------------------------
int MotorManager::disableMotors() {
  if (!myPort) {
    std::cerr << "[MotorManager] No port open.\n";
    return -1;
  }
  try {
    for (size_t i = 0; i < myPort->NodeCount(); ++i) {
      myPort->Nodes(i).EnableReq(false);
      std::cout << "[MotorManager] Node " << i << " disabled.\n";
    }
  } catch (mnErr &e) {
    std::cerr << "[MotorManager] disableMotors error: " << e.ErrorMsg << "\n";
    return -1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
// homeMotors
//   Initiates the homing sequence for all nodes and blocks until complete
//   (up to 30 seconds per axis).
// ---------------------------------------------------------------------------
int MotorManager::homeMotors() {
  if (!myPort) {
    std::cerr << "[MotorManager] No port open.\n";
    return -1;
  }
  try {
    for (size_t i = 0; i < myPort->NodeCount(); ++i) {
      INode &node = myPort->Nodes(i);

      if (!node.Motion.Homing.HomingValid()) {
        std::cerr << "[MotorManager] Node " << i
                  << " has no valid homing config. Skipping.\n";
        continue;
      }

      std::cout << "[MotorManager] Homing node " << i << "...\n";
      node.Motion.Homing.Initiate();

      bool homed =
          waitFor([&node]() { return node.Motion.Homing.WasHomed(); }, 30000);

      if (!homed) {
        std::cerr << "[MotorManager] Node " << i << " homing timed out.\n";
        return -1;
      }
      std::cout << "[MotorManager] Node " << i << " homed.\n";
    }
  } catch (mnErr &e) {
    std::cerr << "[MotorManager] homeMotors error: " << e.ErrorMsg << "\n";
    return -1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
// setSetpoint
//   Stores a desired position (in counts) for the given motor index.
// ---------------------------------------------------------------------------
int MotorManager::setSetpoint(int motorIdx, double setpoint) {
  try {
    // Validate index early so we get a clear error
    getNode(motorIdx);
  } catch (std::exception &e) {
    std::cerr << "[MotorManager] setSetpoint: " << e.what() << "\n";
    return -1;
  }

  if (motorIdx == 0) {
    motor1Setpoint = static_cast<int>(std::round(setpoint));
  } else if (motorIdx == 1) {
    motor2Setpoint = static_cast<int>(std::round(setpoint));
  } else {
    std::cerr << "[MotorManager] setSetpoint: only motor indices 0 and 1 "
                 "supported.\n";
    return -1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
// goToSetpoint
//   Commands both motors to move to their stored setpoints simultaneously.
//   Returns once the moves have been issued (non-blocking); use atSetpoint()
//   to poll for completion.
// ---------------------------------------------------------------------------
int MotorManager::goToSetpoint() {
  if (!myPort) {
    std::cerr << "[MotorManager] No port open.\n";
    return -1;
  }
  try {
    if (Motor1 && Motor1->Motion.IsReady()) {
      Motor1->Motion.MovePosnStart(motor1Setpoint, true /*absolute*/);
      std::cout << "[MotorManager] Motor1 moving to " << motor1Setpoint
                << " counts.\n";
    }
    if (Motor2 && Motor2->Motion.IsReady()) {
      Motor2->Motion.MovePosnStart(motor2Setpoint, true /*absolute*/);
      std::cout << "[MotorManager] Motor2 moving to " << motor2Setpoint
                << " counts.\n";
    }
  } catch (mnErr &e) {
    std::cerr << "[MotorManager] goToSetpoint error: " << e.ErrorMsg << "\n";
    return -1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
// atSetpoint
//   Returns 1 if all active motors have reached their target position, 0 if
//   still moving, and -1 on error.
// ---------------------------------------------------------------------------
int MotorManager::atSetpoint() {
  if (!myPort)
    return -1;
  try {
    for (size_t i = 0; i < myPort->NodeCount(); ++i) {
      INode &node = myPort->Nodes(i);
      if (node.Motion.MoveIsDone()) {
        node.Motion.MoveWentDone(); // Clear the Move done rising edge flag
      } else {
        return 0; // If any node is not done, return 0
      }
    }
  } catch (mnErr &e) {
    std::cerr << "[MotorManager] atSetpoint error: " << e.ErrorMsg << "\n";
    return -1;
  }
  return 1;
}

// ---------------------------------------------------------------------------
// getPosition
//   Returns the current measured position (in counts) for the given motor.
// ---------------------------------------------------------------------------
double MotorManager::getPosition(int motorIdx) {
  try {
    INode *node = getNode(motorIdx);
    return node->Motion.PosnMeasured.Value();
  } catch (std::exception &e) {
    std::cerr << "[MotorManager] getPosition: " << e.what() << "\n";
    return 0.0;
  } catch (mnErr &e) {
    std::cerr << "[MotorManager] getPosition error: " << e.ErrorMsg << "\n";
    return 0.0;
  }
}

// ---------------------------------------------------------------------------
// homeMotor
//   Homes a single node identified by motorIdx.  Returns 0 on success,
//   -1 if the index is invalid, homing is not configured, or times out.
// ---------------------------------------------------------------------------
int MotorManager::homeMotor(int motorIdx) {
  try {
    INode *node = getNode(motorIdx);

    if (!node->Motion.Homing.HomingValid()) {
      std::cerr << "[MotorManager] homeMotor: node " << motorIdx
                << " has no valid homing config (set up in ClearView first).\n";
      return -1;
    }

    std::cout << "[MotorManager] Homing node " << motorIdx << "...\n";
    node->Motion.Homing.Initiate();

    bool homed =
        waitFor([node]() { return node->Motion.Homing.WasHomed(); }, 30000);

    if (!homed) {
      std::cerr << "[MotorManager] homeMotor: node " << motorIdx
                << " timed out.\n";
      return -1;
    }

    node->Motion.PosnMeasured.Refresh();
    std::cout << "[MotorManager] Node " << motorIdx
              << " homed. Position: " << node->Motion.PosnMeasured.Value()
              << " counts.\n";
  } catch (std::exception &e) {
    std::cerr << "[MotorManager] homeMotor: " << e.what() << "\n";
    return -1;
  } catch (mnErr &e) {
    std::cerr << "[MotorManager] homeMotor error: " << e.ErrorMsg << "\n";
    return -1;
  }
  return 0;
}
// ---------------------------------------------------------------------------
// setMotorLimits
//   Applies acceleration and velocity limits to every node on the port.
// ---------------------------------------------------------------------------
int MotorManager::setMotorLimits(double accLimRpmPerSec, double velLimRpm,
                                 double softLimit1, double softLimit2) {
  if (!myPort) {
    std::cerr << "[MotorManager] No port open.\n";
    return -1;
  }
  try {
    for (size_t i = 0; i < myPort->NodeCount(); ++i) {
      INode &node = myPort->Nodes(i);
      node.Motion.AccLimit = accLimRpmPerSec;
      node.Motion.VelLimit = velLimRpm;
      node.Limits.SoftLimit1 = softLimit1;
      node.Limits.SoftLimit2 = softLimit2;
      std::cout << "[MotorManager] Node " << i
                << " limits set: acc=" << accLimRpmPerSec
                << " RPM/s, vel=" << velLimRpm << " RPM "
                << " softLimit1=" << softLimit1 << " softLimit2=" << softLimit2
                << ".";
    }
  } catch (mnErr &e) {
    std::cerr << "[MotorManager] setMotorLimits error: " << e.ErrorMsg << "\n";
    return -1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
// isReady
//   Returns 1 if all nodes are motion-ready, 0 if not, -1 on error.
// ---------------------------------------------------------------------------
int MotorManager::isReady() {
  if (!myPort)
    return 0;
  try {
    for (size_t i = 0; i < myPort->NodeCount(); ++i) {
      if (!myPort->Nodes(i).Motion.IsReady())
        return 0;
    }
  } catch (mnErr &e) {
    std::cerr << "[MotorManager] isReady error: " << e.ErrorMsg << "\n";
    return -1;
  }
  return 1;
}

// ---------------------------------------------------------------------------
// getNodeCount
//   Returns the number of nodes on the open port, or 0 if no port is open.
// ---------------------------------------------------------------------------
int MotorManager::getNodeCount() {
  if (!myPort)
    return 0;
  return static_cast<int>(myPort->NodeCount());
}

// ---------------------------------------------------------------------------
// shutdown
//   Disables all motors and closes all ports. Safe to call multiple times.
// ---------------------------------------------------------------------------
int MotorManager::shutdown() {
  try {
    if (myPort) {
      disableMotors();
    }
    if (myMgr) {
      myMgr->PortsClose();
      std::cout << "[MotorManager] Ports closed.\n";
    }
    myPort = nullptr;
    Motor1 = nullptr;
    Motor2 = nullptr;
  } catch (mnErr &e) {
    std::cerr << "[MotorManager] shutdown error: " << e.ErrorMsg << "\n";
    return -1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
// testHello  – minimal smoke-test
// ---------------------------------------------------------------------------
void MotorManager::testHello() {
  std::cout << "[MotorManager] Hello from TeknicMotors!\n";
  if (myPort) {
    std::cout << "  Port state : " << myPort->OpenState() << "\n";
    std::cout << "  Node count : " << myPort->NodeCount() << "\n";
  } else {
    std::cout << "  (no port open)\n";
  }
}
