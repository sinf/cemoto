from math import pi

wheel_d = 12 * 0.0254
wheel_r = wheel_d / 2
veh_speed_lo = 50 / 3.6
veh_speed_hi = 60 / 3.6
pulses_per_rev = 13

def calc(lin_vel):
    ang_vel = lin_vel / wheel_r
    f = ang_vel / (2*pi)
    fp = f * pulses_per_rev
    print("linear velocity =", lin_vel, "m/s or", lin_vel*3.6, "km/h")
    print("angular velocity =", ang_vel, "radian/s")
    print("angular frequency =", f, "rev/s")
    print("pulse train frequency =", fp, "Hz")
    print()


calc(veh_speed_lo)
calc(veh_speed_hi)

