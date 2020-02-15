import bottle
from cassandra.cluster import Cluster
from cassandra.query import BatchStatement
import bson.json_util as bson_json
from datetime import datetime
from bottle import request, response, get, post

app = application = bottle.Bottle()
application = bottle.default_app()

bottle_host = '192.168.0.27'
bottle_port = 8080

cluster = Cluster()
session = cluster.connect()
session.set_keyspace("fitness")


@get('/account/<rfid>')
#Return the account based on the RFID card; NOT used by Arduino
def get_account(rfid):
	#/account/890
    rows = session.execute('SELECT JSON account FROM account WHERE rfid = ' + rfid)
    for user_row in rows:
    	output_account = user_row[0]
    query_dict = bson_json.loads(output_account)
    return query_dict


@get('/accounts')
#Return all account and the RFID numbers; Not used by Arduino
def get_accounts():
    rows = session.execute('SELECT JSON * FROM account')
    dict = []
    for user_row in rows:
    	dict.append(bson_json.loads((user_row)[0]))
    response.content_type = 'application/json'
    return bson_json.dumps(dict)


@get('/calibration/<account>/<machine>')
#Return the calibration values for a specified account and machine;  Not used by Arduino
def route_index2(account, machine):
	#/calibration/bekama/4
    rows = session.execute('SELECT JSON calibr, restposition FROM calibration WHERE account = %s AND machine =%s', (account, int(machine),))
    for user_row in rows:
    	output = user_row[0]
    	query_dict = bson_json.loads(output)
    return query_dict


@post('/calibration')
#Post the calibration measurement; Used by Arduino at the end of the Calibration process
def calibration():
	#Post the account, machine, restPosition and calibr via JSON
    response.content_type = 'application/json'
    data = str(request.body.read()).replace('b', '', 1)
    print(data)
    session.execute("INSERT INTO calibration JSON" + data)
    return 'Posted'


@get('/sessions/<rfid>/<machine>')
#Checkin and return the account and set number; Not used by Arduino
#sessions/890/4
def sessions(rfid, machine):
	time = datetime.now().strftime('%H:%M:%S')
	acc = session.execute('SELECT JSON account FROM account WHERE rfid = %s LIMIT 1', (int(rfid),))
	for user_row in acc:
		account = bson_json.loads((user_row)[0])
	session.execute("INSERT INTO sessions(machine, account, date, time) VALUES (%s, %s, todate(now()),%s)", (int(machine), account["account"], time,))
	set = session.execute("SELECT JSON COUNT(machine) as sets FROM sessions WHERE machine= %s AND account =%s  AND date=todate(now())", (int(machine), account["account"],))
	for user_row in set:
		output = user_row[0]
		query_dict2 = bson_json.loads(output)
		account.update(query_dict2)
		#print(account)
		return account


@post('/checkin/<rfid>/<machine>')
#Checkin and return the account, set number and calibr values. Used by Arduino once RFID card is registed during checkin process
#checkin/890/4
def checkin(rfid, machine):
	time = datetime.now().strftime('%H:%M:%S')
	acc = session.execute('SELECT JSON account FROM account WHERE rfid = %s LIMIT 1', (int(rfid),))
	for user_row in acc:
		account = bson_json.loads((user_row)[0])
	session.execute("INSERT INTO sessions(machine, account, date, time,status) VALUES (%s, %s, todate(now()),%s,'in')", (int(machine), account["account"], time,))
	set = session.execute("SELECT JSON COUNT(machine) as sets FROM sessions WHERE machine= %s AND status='in' AND account =%s  AND date=todate(now()) ALLOW FILTERING", (int(machine), account["account"],))
	for user_row in set:
		output = user_row[0]
		query_dict2 = bson_json.loads(output)
		account.update(query_dict2)
	calib = session.execute('SELECT JSON calibr, restposition FROM calibration WHERE account = %s AND machine =%s', (account["account"], int(machine),))
	for user_row in calib:
		output2 = user_row[0]
		query_calibr = bson_json.loads(output2)
	account.update(query_calibr)
	#print(account)
	return account


list = ['{"account": "hworangs", "machine": 4, "date": "2018-09-09", "sets": 99, "rep": 3, "contraction": 980, "extension": 800, "power": 90, "weight": 80}', '{"account": "hworangs", "machine": 4, "date": "2018-09-09", "sets": 99, "rep": 4, "contraction": 980, "extension": 800, "power": 90, "weight": 80}']


@post('/checkout/<account>/<machine>')
#Checkout to finish a set; Used by Arduino at the end of the training once the training is finished
#checkout/hworangs/4
def checkout(account, machine):
	time = datetime.now().strftime('%H:%M:%S')
	session.execute("INSERT INTO sessions(machine, account, date, time,status) VALUES (%s, %s, todate(now()),%s,'out')", (int(machine), account, time,))
	#Parse the incoming JSON array and loop thru them and add into the batch
	response.content_type = 'application/json'
	#data = str(request.body.read()).replace('b', '', 1) #WORKING
	data = bson_json.loads(request.body.read())
	#print(data)

	batch = BatchStatement()
	for json in data:      #FIND A WAY TO re-write the data as in list above or array of json
		print(json)
		batch.add("""Insert INTO training_details JSON \'""" + json + "\'")
	session.execute(batch)
	return 'Finished set!'


@post('/account/<rfid>/<account>')
#Add a new account with the RFID number provided; NOT used by Arduino; but via the webpage for create new account
def post_account(rfid, account):
	#/account/112/gaga
    session.execute("INSERT into account(rfid,account) VALUES (%s, %s)", (int(rfid), account))
    return "New account added"


@get('/training_details/<account>/<machine>/<date>')
#Receive the details for each training
#/training_details/hworangs/4/2018-10-23
def get_training_details(account, machine, date):
	rows = session.execute("Select JSON * from training_details WHERE account= %s and machine=%s and date= %s", (account, int(machine), date,))
	dict = []
	for user_row in rows:
		dict.append(bson_json.loads((user_row)[0]))
		response.content_type = 'application/json'
	return bson_json.dumps(dict)

bottle.run(host=bottle_host, port=bottle_port, reloader=bottle.DEBUG)