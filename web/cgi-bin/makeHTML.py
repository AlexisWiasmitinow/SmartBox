import time
import datetime

def sendResponse():
	#now=time.time()
	now=datetime.datetime.now().strftime("%d.%m %H:%M:%S")
	answer="<h1>Hello World From Python, it is:"+str(now)+"</h1>"
	return answer