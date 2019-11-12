from multiprocessing import Process, Pipe
import threading
import random
import time


eleStates = ['up', 'down', 'stop', 'open', 'close']

class Elevator(Process):
	def __init__(self, floorNum, pipes):
		super(Elevator, self).__init__()
		self.fn = floorNum
		self.curFloor = random.randint(1,floorNum)
		self.state = 'stop'
		self.pipes = pipes
		self.upFloors = [False for i in range(floorNum)]
		self.downFloors = [False for i in range(floorNum)]

	def run(self):
		print("I am elevator.")
		while(False):
			# read ctrl pad, update usr cmd
			pass
			# move
			# send self state 

class eleCtrlPad(Process):
	def __init__(self, floorNum, pipe):
		super(eleCtrlPad, self).__init__()
		self.fn = floorNum
		self.floorBtns = [False for i in range(floorNum)]
		self.openBtn = False
		self.closeBtn = False
		self.curFloor = None
		self.pipe = pipe

	def run(self):
		print("this is control pad of elevator")
		# recv usr cmd, send to ele
		# show ele state


class floorCtrlPad(Process):
	def __init__(self, floor, pipe):
		super(floorCtrlPad, self).__init__()
		self.floor = floor
		self.upBtn = False
		self.downBtn = False
		self.curFloor = None
		self.pipe = pipe

	def run(self):
		print("this is control pad of floor " + str(self.floor))
		
class screen(threading.Thread):
	def __init__(self):
		super(screen, self).__init__()

	def run(self):
		# show els state
		pass

class elePad(threading.Thread):
	def __init__(self):
		super(elePad, self).__init__()
	
	def run(self):
		# recv usr cmd, send to ele
		pass

class floorPad(threading.Thread):
	def __init__(self):
		super(floorPad, self).__init__()

	def run(self):
		# recv usr cmd, send to ele
		pass


if __name__ == "__main__":
	n = 3

	# need 4 pipes, 3 for floors and 1 for eleCtrlPad
	pipes = []
	for i in range(n+1):
		pipes.append(Pipe())

	# create processes
	ele = Elevator(n, [pipes[i][0] for i in range(n+1)])
	ecp = eleCtrlPad(n, pipes[0][1])
	fcp = []
	for i in range(1, 1+n):
		fcp.append(floorCtrlPad(i, pipes[i][1]))
		fcp[i-1].start()
	ele.start()
	ecp.start()
	
	
