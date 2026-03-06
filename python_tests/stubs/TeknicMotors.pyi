"""

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
"""
from __future__ import annotations
import typing
__all__: list[str] = ['MotorManager']
class MotorManager:
    """
    
    Controls one or more Teknic ClearPath-SC motors connected through an SC Hub.
    
    All methods return an integer status code (0 = success, non-zero = failure)
    unless documented otherwise.
    """
    def __init__(self) -> None:
        """
        Construct a MotorManager with no open connections.
        """
    def at_setpoint(self) -> int:
        """
        Check whether all active motors have reached their target positions.
        
        Returns
        -------
        int
            1 if all motors are at their targets, 0 if still moving, -1 on error.
        """
    def disable_motors(self) -> int:
        """
        Send a disable request to every node on the open port.
        
        Returns
        -------
        int
            0 on success, -1 on error.
        """
    def enable_motors(self) -> int:
        """
        Send an enable request to every node on the open port and wait up to 3 s for
        them to become motion-ready.
        
        Returns
        -------
        int
            0 on success, -1 on timeout or error.
        """
    def find_and_open_ports(self) -> int:
        """
        Scan for SC Hub COM ports and open the first one found.
        
        Populates the internal port and node references used by all subsequent calls.
        
        Returns
        -------
        int
            0 on success, -1 if no hub is found or the port cannot be opened.
        """
    def get_node_count(self) -> int:
        """
        Return the number of nodes found on the open port.
        
        Returns
        -------
        int
            Node count, or 0 if no port is open.
        """
    def get_position(self, motor_idx: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
        Return the current measured position (encoder counts) of one motor.
        
        Parameters
        ----------
        motor_idx : int
            0-based index of the motor.
        
        Returns
        -------
        float
            Current position in encoder counts, or 0.0 on error.
        """
    def go_to_setpoint(self) -> int:
        """
        Command both motors to move to their stored setpoints simultaneously.
        
        The call returns immediately after issuing the commands; poll
        ``at_setpoint()`` to detect completion.
        
        Returns
        -------
        int
            0 on success, -1 on error.
        """
    def home_motors(self) -> int:
        """
        Initiate the homing sequence for every node that has a valid homing
        configuration (set up in Teknic ClearView) and block until complete.
        
        Each axis is given up to 30 seconds to complete homing.
        
        Returns
        -------
        int
            0 on success, -1 if any axis times out.
        """
    def initialize(self) -> int:
        """
        Create the internal SysManager instance.
        
        Must be called before any other method.
        
        Returns
        -------
        int
            0 on success, -1 on failure.
        """
    def is_ready(self) -> int:
        """
        Check whether all nodes are motion-ready.
        
        Returns
        -------
        int
            1 if all nodes are ready, 0 if not, -1 on error.
        """
    def set_motor_limits(self, acc_rpm_per_sec: typing.SupportsFloat | typing.SupportsIndex = 1000.0, vel_rpm: typing.SupportsFloat | typing.SupportsIndex = 100.0) -> int:
        """
        Set acceleration and velocity limits on every node.
        
        Parameters
        ----------
        acc_rpm_per_sec : float
            Acceleration limit in RPM/s (default 1000).
        vel_rpm : float
            Velocity limit in RPM (default 100).
        
        Returns
        -------
        int
            0 on success, -1 on error.
        """
    def set_setpoint(self, motor_idx: typing.SupportsInt | typing.SupportsIndex, setpoint: typing.SupportsFloat | typing.SupportsIndex) -> int:
        """
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
        """
    def shutdown(self) -> int:
        """
        Disable all motors and close all open ports.
        
        Safe to call multiple times.
        
        Returns
        -------
        int
            0 on success, -1 on error.
        """
    def test_hello(self) -> None:
        """
        Print a brief status summary to stdout (useful for smoke-testing).
        """
