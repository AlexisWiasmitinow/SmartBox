#!/usr/bin/env python
#print("hello world in python")
import time
import sys
import cgi
import cgitb
cgitb.enable()
sys.path.append( "/home/pi/SmartBox/web/cgi-bin")
from makeHTML import *
def application(env, start_response):
	start_response('200 OK', [('Content-Type','text/html')])
	answer=sendResponse(env['REQUEST_URI'])
	#answer="Hello World From Python, it is: "
	return answer
    