import TeknicMotors

mgr = TeknicMotors.MotorManager()
_ = mgr.initialize()
_ = mgr.find_and_open_ports()
_ = mgr.set_motor_limits(acc_rpm_per_sec=500.0, vel_rpm=200.0)
_ = mgr.enable_motors()
_ = mgr.home_motors()
_ = mgr.set_setpoint(motor_idx=0, setpoint=2000.0)
_ = mgr.set_setpoint(motor_idx=1, setpoint=1500.0)
_ = mgr.go_to_setpoint()
while not mgr.at_setpoint():
    pass
print("Motor 0 position:", mgr.get_position(0))
print("Motor 1 position:", mgr.get_position(1))
mgr.shutdown()