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
		self.insideFloors = [False for i in range(floorNum)]

	def run(self):
		print("I am elevator.")
		while(True):
			time.sleep(1)
			# read ctrl pad, update usr cmd
			if self.pipes[0].poll():
				self.insideFloors = self.pipes[0].recv()
				# print(self.insideFloors)			

			for i in range(1,self.fn+1):
				if self.pipes[i].poll():
					self.upFloors[i-1], self.downFloors[i-1] = self.pipes[i].recv()
					# print(self.upFloors[i-1],self.downFloors[i-1])
			# move
			# send self state 
			for p in self.pipes:
				p.send([self.curFloor, self.state])
						

class eleCtrlPad(Process):
	def __init__(self, floorNum, pipe):
		super(eleCtrlPad, self).__init__()
		self.fn = floorNum
		self.floorBtns = [False for i in range(floorNum)]
		self.curFloor = [None]
		self.pipe = pipe

	def run(self):
		print("this is control pad of elevator")
		mutex = threading.RLock()
		s = screen(mutex, self.pipe, self.curFloor)
		p = elePad(mutex, self.pipe, self.curFloor, self.floorBtns)
		s.start()
		p.start()
		s.join()
		p.join()
		

class floorCtrlPad(Process):
	def __init__(self, floor, pipe):
		super(floorCtrlPad, self).__init__()
		self.floor = floor
		self.btns = [False, False]
		self.curFloor = [None]
		self.pipe = pipe

	def run(self):
		print("this is control pad of floor " + str(self.floor))
		mutex = threading.RLock()

		s = screen(mutex, self.pipe, self.curFloor)
		p = floorPad(mutex, self.pipe, self.floor, self.curFloor, self.btns)
		s.start()
		p.start()
		s.join()
		p.join()		


class screen(threading.Thread):
	def __init__(self, mutex, pipe, curFloor):
		super(screen, self).__init__()
		self.mutex = mutex
		self.pipe = pipe
		self.curFloor = curFloor

	def run(self):
		### recv ele state, show els state
		while(True):
			time.sleep(1)
			if self.mutex.acquire():
				if self.pipe.poll():
					self.curFloor[0],state = self.pipe.recv()
					print("elevator on %s floor, is %s"%(self.curFloor[0], state))
					self.mutex.release()


class elePad(threading.Thread):
	def __init__(self, mutex, pipe, curFloor, btns):
		super(elePad, self).__init__()
		self.mutex = mutex
		self.pipe = pipe
		self.curFloor = curFloor
		self.btns = btns
		self.mem = [b for b in btns]
	
	def btnChanged(self):
		for i in range(len(self.btns)):
			if self.mem[i] != self.btns[i]:
				self.mem[i] = self.btns[i]
				return True
		return False

	def run(self):
		time.sleep(1)
		if self.mutex.acquire():
			self.pipe.send(self.btns)
			self.mutex.release()
		while(True):
			time.sleep(1)
			try:
				self.btns[self.curFloor[0]-1] = False
			except IndexError:
				print("curFloor: %s"%self.curFloor[0])
			if self.btnChanged():
				if self.mutex.acquire():
					self.pipe.send(self.btns)
					self.mutex.release()


class floorPad(threading.Thread):
	def __init__(self, mutex, pipe, floor, curFloor, btns):
		super(floorPad, self).__init__()
		self.mutex = mutex
		self.pipe = pipe
		self.floor = floor
		self.curFloor = curFloor
		self.btns = btns
		self.mem = [b for b in btns]
	
	def btnChanged(self):
		for i in range(len(self.btns)):
			if self.mem[i] != self.btns[i]:
				self.mem[i] = self.btns[i]
				return True
		return False

	def run(self):
		if self.mutex.acquire():
			self.pipe.send(self.btns)
			self.mutex.release()
		while(True):
			time.sleep(1)
			if self.curFloor[0] == self.floor:
				self.btns = [False, False]
			if self.btnChanged():
				if self.mutex.acquire():
					self.pipe.send(self.btns)
					self.mutex.release()



if __name__ == "__main__":
	n = 3

	# need 4 pipes, 3 for floors and 1 for eleCtrlPad
	pipes = []
	pps = []
	for i in range(n+1):
		pipes.append(Pipe())
		pps.append(pipes[i][0])

	# create processes
	ele = Elevator(n, pps)
	ecp = eleCtrlPad(n, pipes[0][1])
	fcp = []
	for i in range(1, 1+n):
		fcp.append(floorCtrlPad(i, pipes[i][1]))
		fcp[i-1].start()
	ele.start()
	ecp.start()
	
	
