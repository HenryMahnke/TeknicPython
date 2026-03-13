import TeknicMotors
from TeknicMotors import MotorManager
import asyncio
import time

mgr: MotorManager = TeknicMotors.MotorManager()
async def setup_motors(): 
    if (mgr.initialize() != 0):
        print("Failed to initialize MotorManager")
        exit(1)

    if (mgr.find_and_open_ports() != 0):
        print("Failed to find and open ports")
        exit(1)

    if (mgr.set_motor_limits(acc_rpm_per_sec=100.0, vel_rpm=100.0) != 0):
        print("Failed to set motor limits")
        exit(1)
async def enable_motors():
    if (mgr.enable_motors() != 0):
        print("Failed to enable motors")
        exit(1)

async def disable_motors():
    _ = mgr.disable_motors()

async def set_setpoint(motor_idx: int, setpoint: float):
    if (mgr.set_setpoint(motor_idx=motor_idx, setpoint=setpoint) != 0):
        print("Failed to set setpoint for motor {motor_idx}")
        exit(1)

# if (mgr.home_motors() != 0):
#     print("Failed to home motors")
#     exit(1)
async def go_to_setpoint():
    if (mgr.go_to_setpoint() != 0):
        print("Failed to go to setpoint")
        exit(1)

# if (mgr.set_setpoint(motor_idx=1, setpoint=1500.0) != 0):
#     print("Failed to set setpoint for motor 1")
#     exit(1)

async def at_setpoint():
    while not mgr.at_setpoint():
        time.sleep(0.05)

async def get_position(motor_idx: int):
    return mgr.get_position(motor_idx=motor_idx)

async def shutdown():
    if (mgr.shutdown() != 0):
        print("Failed to shutdown MotorManager")
        exit(1)

async def main():
    print("STARTING TEST")
    await asyncio.sleep(2)
    await setup_motors()
    print("setup motors was successful")
    await enable_motors()
    position = await get_position(motor_idx=0)
    position1 = await get_position(motor_idx=1)
    print(f"Motor 0 position: {position}")
    print(f"Motor 1 position: {position1}")

    await set_setpoint(motor_idx=0, setpoint=8000.0)
    await set_setpoint(motor_idx=1, setpoint=8000.0)
    print("about to go to setpoint")
    await asyncio.sleep(3)
    await go_to_setpoint()
    await at_setpoint()
    position = await get_position(motor_idx=0)
    position1 = await get_position(motor_idx=1)
    print(f"Motor 0 position: {position}")
    print(f"Motor 1 position: {position1}")
    await set_setpoint(motor_idx=0, setpoint=16000.0)
    await set_setpoint(motor_idx=1, setpoint=16000.0)
    await go_to_setpoint()
    await at_setpoint()
    await asyncio.sleep(3)
    await set_setpoint(motor_idx=0, setpoint=24000.0)
    await set_setpoint(motor_idx=1, setpoint=24000.0)
    await go_to_setpoint()
    await at_setpoint()

    await asyncio.sleep(3)
    await set_setpoint(motor_idx=0, setpoint=32000.0)
    await set_setpoint(motor_idx=1, setpoint=32000.0)
    await go_to_setpoint()
    await at_setpoint()
    
    await asyncio.sleep(3)
    await set_setpoint(motor_idx=0, setpoint=40000.0)
    await set_setpoint(motor_idx=1, setpoint=40000.0)
    await go_to_setpoint()
    await at_setpoint()

    await shutdown()

if __name__ == "__main__":
    asyncio.run(main())