#!/usr/bin/env python
import smbus
import time
import sys

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

if __name__ == '__main__': # main function
	argv = sys.argv
	if len(argv) < 2 or len(argv) > 3:
		print("\nUsage: %s [ -r] [ -d] [ -s VNEG]\n"%(argv[0]))
		sys.exit(-1)
	bus = smbus.SMBus(1)
	time.sleep(0.1)
	st = 0
	if argv[1] == '-r' and len(argv) == 2: # reset
		print("Resetting chip...")
		st = block_write(bus, 0x45, 0x4, 0b00100000)
		if st != 0:
			print("Reset not successful: %d"%(st))
			sys.exit(-1)
		
		print("Enabling register input...")
		st = block_write(bus, 0x45, 0x4, 0x7)
		if st != 0x7:
			print("Bus enable unsuccessful: %d"%(st))
			sys.exit(-2)
		
		print("Setting output voltage to -8V")
		st = block_write(bus, 0x45, 0x1, 0x88)
		if st != 0x88:
			print("Could not set output voltage to -8V: %d"%(st))
			sys.exit(-3)
		
		print("Enabling output")
		st = block_write(bus, 0x45, 0x2, 0x3)
		if st != 0x3:
			print("Could not enable outputs: %d"%(st))
			sys.exit(-4)
		
		sys.exit(1)

	elif argv[1] == '-d' and len(argv) == 2: # default
		# check if chip is set up to read inputs from registers
		st = block_read(bus, 0x45, 0x4)
		retry = 10
		while st != 0x7 and retry > 0:
			st = block_write(bus, 0x45, 0x4, 0x7)
			retry -= 1
		if retry == 0 and st != 0x7:
			sys.exit(-5)
		# check if chip is set to output
		st = block_read(bus, 0x45, 0x2)
		retry = 10
		while st != 0x3 and retry > 0:
			st = block_write(bus, 0x45, 0x2, 0x3)
			retry -= 1
		if retry == 0 and st != 0x3:
			sys.exit(-6)
		
		st = block_read(bus, 0x45, 0x1)
		retry = 10
		while st != 0x88 and retry > 0:
			st = block_write(bus, 0x45, 0x1, 0x88)
			retry -= 1
		if retry == 0 and st != 0x88:
			sys.exit(-7)
		
		sys.exit(1)
		
	elif argv[1] == '-s' and len(argv) == 3: # set voltage
		tv = 0.0
		try:
			tv = float(argv[2])
		except Exception: # TypeError or something similar
			print("Input %s not a valid floating point number"%(argv[2]))
			sys.exit(-8)
		
		if (tv < -13.95) or ((tv + 1.2) > 0):
			print("%.3f not in valid range (-13.95, -1.2)"%(tv))
			sys.exit(-9)

		cmd = int((tv + 1.2)/-0.05)
		if cmd < 0 or cmd > 255:
			print("Invalid command %d"%(cmd))
			sys.exit(-10)
		
		st = block_write(bus, 0x45, 0x1, cmd)

		if st != cmd:
			print("Could not set %d, got %d"%(cmd, st))
			sys.exit(-11)
		
		sys.exit(1)

	else: # invalid option combo
		print("Invalid options", argv)
		sys.exit(-127)
