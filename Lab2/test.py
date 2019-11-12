import time
import threading

def run():
	print("!!")
	time.sleep(1)
	print("!")


print("!!!")
t = threading.Thread(target = run, args = ())
t.start()

t.join()
