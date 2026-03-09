// bindings.cpp – pybind11 module exposing the MotorManager C++ class to Python
//
// After building with CMake the resulting shared library can be imported as:
//
//   import TeknicMotors
//   mgr = TeknicMotors.MotorManager()
//   mgr.initialize()
//   mgr.find_and_open_ports()
//   mgr.set_motor_limits(500.0, 200.0)
//   mgr.enable_motors()
//   mgr.home_motors()
//   mgr.set_setpoint(0, 2000.0)
//   mgr.go_to_setpoint()
//   while not mgr.at_setpoint():
//       pass
//   pos = mgr.get_position(0)
//   mgr.shutdown()

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "TeknicMotors.hpp"

namespace py = pybind11;

PYBIND11_MODULE(TeknicMotors, m) {
    m.doc() = R"doc(
TeknicMotors
============
Python bindings for the Teknic ClearPath-SC sFoundation motor control library.

The central object is ``MotorManager``, which wraps all interactions with the
SC Hub and the nodes (motors) attached to it.

Typical usage
-------------
::

    import TeknicMotors

    mgr = TeknicMotors.MotorManager()
    mgr.initialize()
    mgr.find_and_open_ports()
    mgr.set_motor_limits(acc_rpm_per_sec=500.0, vel_rpm=200.0)
    mgr.enable_motors()
    mgr.home_motors()

    mgr.set_setpoint(motor_idx=0, setpoint=2000.0)
    mgr.set_setpoint(motor_idx=1, setpoint=1500.0)
    mgr.go_to_setpoint()

    import time
    while not mgr.at_setpoint():
        time.sleep(0.05)

    print("Motor 0 position:", mgr.get_position(0))
    mgr.shutdown()
)doc";

    py::class_<MotorManager>(m, "MotorManager",py::call_guard<py::gil_scoped_release>(),
        R"doc(
Controls one or more Teknic ClearPath-SC motors connected through an SC Hub.

All methods return an integer status code (0 = success, non-zero = failure)
unless documented otherwise.
)doc")

        .def(py::init<>(),
             "Construct a MotorManager with no open connections.")

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------
        .def("initialize", &MotorManager::inititalize,py::call_guard<py::gil_scoped_release>(),
             R"doc(
Create the internal SysManager instance.

Must be called before any other method.

Returns
-------
int
    0 on success, -1 on failure.
)doc")

        .def("find_and_open_ports", &MotorManager::findAndOpenPorts,py::call_guard<py::gil_scoped_release>(),
             R"doc(
Scan for SC Hub COM ports and open the first one found.

Populates the internal port and node references used by all subsequent calls.

Returns
-------
int
    0 on success, -1 if no hub is found or the port cannot be opened.
)doc")

        .def("enable_motors", &MotorManager::enableMotors,py::call_guard<py::gil_scoped_release>(),
             R"doc(
Send an enable request to every node on the open port and wait up to 3 s for
them to become motion-ready.

Returns
-------
int
    0 on success, -1 on timeout or error.
)doc")

        .def("disable_motors", &MotorManager::disableMotors,py::call_guard<py::gil_scoped_release>(),
             R"doc(
Send a disable request to every node on the open port.

Returns
-------
int
    0 on success, -1 on error.
)doc")

        .def("home_motors", &MotorManager::homeMotors,py::call_guard<py::gil_scoped_release>(),
             R"doc(
Initiate the homing sequence for every node that has a valid homing
configuration (set up in Teknic ClearView) and block until complete.

Each axis is given up to 30 seconds to complete homing.

Returns
-------
int
    0 on success, -1 if any axis times out.
)doc")

        .def("shutdown", &MotorManager::shutdown,
             R"doc(
Disable all motors and close all open ports.

Safe to call multiple times.

Returns
-------
int
    0 on success, -1 on error.
)doc")

        // ------------------------------------------------------------------
        // Motion
        // ------------------------------------------------------------------
        .def("set_setpoint", &MotorManager::setSetpoint,py::call_guard<py::gil_scoped_release>(),
             py::arg("motor_idx"), py::arg("setpoint"),
             R"doc(
Store a desired absolute position (in encoder counts) for one motor.

The move is not executed until ``go_to_setpoint()`` is called.

Parameters
----------
motor_idx : int
    0-based index of the target motor (0 or 1).
setpoint : float
    Target absolute position in encoder counts.

Returns
-------
int
    0 on success, -1 on invalid index.
)doc")

        .def("go_to_setpoint", &MotorManager::goToSetpoint,py::call_guard<py::gil_scoped_release>(),
             R"doc(
Command both motors to move to their stored setpoints simultaneously.

The call returns immediately after issuing the commands; poll
``at_setpoint()`` to detect completion.

Returns
-------
int
    0 on success, -1 on error.
)doc")

        .def("at_setpoint", &MotorManager::atSetpoint,py::call_guard<py::gil_scoped_release>(),
             R"doc(
Check whether all active motors have reached their target positions.

Returns
-------
int
    1 if all motors are at their targets, 0 if still moving, -1 on error.
)doc")

        .def("get_position", &MotorManager::getPosition,py::call_guard<py::gil_scoped_release>(),
             py::arg("motor_idx"),
             R"doc(
Return the current measured position (encoder counts) of one motor.

Parameters
----------
motor_idx : int
    0-based index of the motor.

Returns
-------
float
    Current position in encoder counts, or 0.0 on error.
)doc")

        // ------------------------------------------------------------------
        // Configuration
        // ------------------------------------------------------------------
        .def("set_motor_limits",
             &MotorManager::setMotorLimits,py::call_guard<py::gil_scoped_release>(),
             py::arg("acc_rpm_per_sec") = 1000.0,
             py::arg("vel_rpm") = 100.0,
             py::arg("soft_limit_1") = 0.0,
             py::arg("soft_limit_2") = 0.0,
             R"doc(
Set acceleration and velocity limits on every node.

Parameters
----------
acc_rpm_per_sec : float
    Acceleration limit in RPM/s (default 1000).
vel_rpm : float
    Velocity limit in RPM (default 100).
soft_limit_1 : float
    Soft limit 1 in encoder counts (default 0).
soft_limit_2 : float
    Soft limit 2 in encoder counts (default 0).

Returns
-------
int
    0 on success, -1 on error.
)doc")

        // ------------------------------------------------------------------
        // Status / info
        // ------------------------------------------------------------------
        .def("is_ready", &MotorManager::isReady,py::call_guard<py::gil_scoped_release>(),
             R"doc(
Check whether all nodes are motion-ready.

Returns
-------
int
    1 if all nodes are ready, 0 if not, -1 on error.
)doc")

        .def("get_node_count", &MotorManager::getNodeCount,py::call_guard<py::gil_scoped_release>(),
             R"doc(
Return the number of nodes found on the open port.

Returns
-------
int
    Node count, or 0 if no port is open.
)doc")

        .def("test_hello", &MotorManager::testHello, py::call_guard<py::gil_scoped_release>(),
             "Print a brief status summary to stdout (useful for smoke-testing).");
}
