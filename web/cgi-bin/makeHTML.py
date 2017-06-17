import time
import datetime
from pymongo import MongoClient
import cgi
import cgitb
cgitb.enable()
def sendResponse(formData):
	
	client = MongoClient("mongodb://127.0.0.1:27017")
	db = client.test
	cursor = db.screws.find_one({'product_id':'2'})
	#now=time.time()
	uri,data = formData.split("?")
	variables = data.split("&")
	now=datetime.datetime.now().strftime("%d.%m %H:%M:%S")
	answer="<h1>Hello World From Python, it is:"+str(now)+"</h1>"
	answer+="<h1>Hello World From mongoDB, Raw:"+str(cursor)+"</h1>"
	answer+="<h1>DIN:"+str(cursor['norms']['DIN'])+"</h1>"
	answer+="<h1>Length:"+str(cursor['dimensions']['length'])+"</h1>"
	for i in variables:
		name,value=i.split("=")
		answer+="<h1>"+str(name)+":"+str(value)+"</h1>"
	answer+="<h1>URI:"+str(formData)+"</h1>"
	#product_id = formData.getvalue('product_id')
	#answer+="<h1>ID:"+str(product_id)+"</h1>"
	return answer