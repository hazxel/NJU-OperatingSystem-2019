from multiprocessing import Process, Pipe
import threading
import random
import time
import os


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
	def haveDownReq(self):
		if self.curFloor == 1:
			return False
		for i in range (1,self.curFloor):
			if self.downFloors[i-1]==True or self.upFloors[i-1]==True or self.insideFloors[i-1]==True:
				return True
		return False

	def haveUpReq(self):
		if self.curFloor == self.fn:
			return False
		for i in range (self.curFloor+1, self.fn+1):
			if self.downFloors[i-1] or self.upFloors[i-1] or self.insideFloors[i-1]:
				return True
		return False

	def haveReq(self):
		return self.insideFloors[self.curFloor] or self.upFloors[self.curFloor] or self.downFloors[self.curFloor]
    
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
                    
			# move
			if self.state == 'stop':
				if self.haveDownReq():
					self.state = 'down'
				elif self.haveUpReq():
					self.state = 'up'
			elif self.state == 'up':
				print("*** Elevator going up from %s to %s..."%(self.curFloor, self.curFloor+1))
				time.sleep(3)
				self.curFloor += 1
				if self.insideFloors[self.curFloor-1] or self.upFloors[self.curFloor-1] or (not self.haveUpReq()):
					self.state = 'stop'
				else:
					self.state = 'up'
			elif self.state == 'down':
				print("*** Elevator going down from %s to %s..."%(self.curFloor, self.curFloor-1))
				time.sleep(3)
				self.curFloor -= 1
				if self.insideFloors[self.curFloor-1] or self.downFloors[self.curFloor-1] or (not self.haveDownReq()):
					self.state = 'stop'
				else:
					self.state = 'down'

			# send self state 
			for p in self.pipes:
				p.send([self.curFloor, self.state])
						

class eleCtrlPad(Process):
	def __init__(self, floorNum, pipe):
		super(eleCtrlPad, self).__init__()
		self.fn = floorNum
		self.floorBtns = [False for i in range(floorNum)]
		self.curFloor = [None]
		self.curState = [None]
		self.pipe = pipe

	def run(self):
		mutex = threading.RLock()
		s = screen(mutex, self.pipe, self.curFloor, self.curState, "inside screen")
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
		self.curState = [None]
		self.pipe = pipe

	def run(self):
		mutex = threading.RLock()

		s = screen(mutex, self.pipe, self.curFloor, self.curState, "F"+str(self.floor)+" screen")
		p = floorPad(mutex, self.pipe, self.floor, self.curFloor, self.curState, self.btns)
		s.start()
		p.start()
		s.join()
		p.join()		


class screen(threading.Thread):
	def __init__(self, mutex, pipe, curFloor, curState, name):
		super(screen, self).__init__()
		self.mutex = mutex
		self.pipe = pipe
		self.curFloor = curFloor
		self.curState = curState
		self.name = name

	def run(self):
		### recv ele state, show els state
		while(True):
			time.sleep(0.1)
			if self.mutex.acquire():
				if self.pipe.poll():
					self.curFloor[0],self.curState[0] = self.pipe.recv()
					print("# %s: elevator on F%s, is %s"%(self.name,self.curFloor[0],self.curState[0]))
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
				print("IndexErrorInfo: curFloor is %s"%self.curFloor[0])
			if self.btnChanged():
				if self.mutex.acquire():
					self.pipe.send(self.btns)
					self.mutex.release()


class floorPad(threading.Thread):
	def __init__(self, mutex, pipe, floor, curFloor, curState, btns):
		super(floorPad, self).__init__()
		self.mutex = mutex
		self.pipe = pipe
		self.floor = floor
		self.curFloor = curFloor
		self.curState = curState
		self.btns = btns
		self.mem = [b for b in btns]
		self.upORdown = 0

	def updateUOD(self):
		if self.curState == 'up':
			self.upORdown = 0
		if self.curState == 'down':
			self.upORdown = 1
	
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
		print("@ F%s btn state: [%s]UP [%s]DOWN"%(self.floor,'*' if self.btns[0] else '','*' if self.btns[1] else ''))
        
		while(True):
			time.sleep(1)
			if self.curFloor[0] == self.floor:
				self.btns[self.upORdown] = False
			if self.btnChanged():
				print("@ F%s btn state: [%s]UP [%s]DOWN"%(self.floor,'*' if self.btns[0] else '','*' if self.btns[1] else ''))
				if self.mutex.acquire(1):
					self.pipe.send(self.btns)
					self.mutex.release()


def initialSetup():
	ecp.floorBtns = [True, False, False]
	fcp[0].btns = [True, False]
	ele.curFloor = 3


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
	
	initialSetup()

	for i in range(1,1+n):
		fcp[i-1].start()
	ele.start()
	ecp.start()
