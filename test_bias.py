#!/usr/bin/env python
import smbus
import time

def block_read(bus, addr, reg):
	retval = int(0)
	while True:
		try:
			retval = bus.read_byte_data(addr, reg)
		except Exception:
			continue
		break
	return retval

def block_write(bus, addr, reg, val):
	while True:
		try:
			bus.write_byte_data(addr, reg, val)
		except Exception:
			continue
		break
	return block_read(bus, addr, reg)

bus = smbus.SMBus(1)
time.sleep(0.1)
# Startup sequence
print("Resetting chip...")
block_write(bus, 0x45, 0x4, 0b00100000)
time.sleep(0.1)
print("After reset: 0x%x"%(block_read(bus, 0x45, 0x4)))
print("Enabling register input")
print("After bus enable: 0x%x"%(block_write(bus, 0x45, 0x4, 0x7)))
time.sleep(0.1)
print("Setting output to -8V")
block_write(bus, 0x45, 0x1, 0x88)
print("Enabling output")
block_write(bus, 0x45, 0x2, 0x3)
while True:
	tv = float(input("Enter target voltage: "))
	if (tv < -13.95) or ((tv + 1.2) > 0):
		continue
	cmd = int((tv + 1.2)/-0.05)
	print("Target command:", cmd)
	print("Set:", block_write(bus, 0x45, 0x1, cmd))
