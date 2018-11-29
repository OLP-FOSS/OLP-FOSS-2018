import paho.mqtt.client as mqttClient
import time
import requests
import json 
import numpy as np
from pandas.io.json import json_normalize

# sklearn externals
from sklearn.externals import joblib
 
Connected = False   #global variable for the state of the connection
client = None

#broker_address= "m15.cloudmqtt.com"  #Broker address
broker_address= "mqtt"  #Broker address
#port = 19468                         #Broker port for mqtt cloud
port = 1883                         #Broker port local host
user = "esp32"                    #Connection username
password = "123456789"            #Connection password

# this is to get data from a response
api_url = "http://nodered:1880/data"

#topic_sub = "detector/sub"
topic_pub = "detector"

# load sklearn model
model = joblib.load('./see_watch_model_2.pkl')

prev = 0

def on_connect(client, userdata, flags, rc):
 
    if rc == 0:
        print("Connected to broker")
        global Connected                #Use global variable
        Connected = True                #Signal connection 
    else:
        print("Connection failed. Exit code = ",rc)
 
def on_message(client, userdata, message):
    print ("Message receive: ", message.payload.decode("utf-8") )
    #client.publish(topic_pub, payload="ret_cool", qos=1, retain=False)

 
def init_connection():
    global client
    client = mqttClient.Client("Python")               #create new instance
    #client.username_pw_set(user, password=password)    #set username and password
    client.on_connect= on_connect                      #attach function to callback
    client.on_message= on_message                      #attach function to callback
    
    try:
        client.connect(broker_address, port=port)          #connect to broker
    except:
        print("Connection mqtt error")

    client.loop_start()        #start the loop
 
    while Connected != True:    #Wait for connection
        time.sleep(1)
    
    # client.subscribe(topic_sub)
    
    # try:
    #     while True:
    #         time.sleep(1)
    
    # except KeyboardInterrupt:
    #     print ("exiting")
    #     client.disconnect()
    #     client.loop_stop()

def get_X_series_concat(data):
    X_test = []
    #data = data.iloc[-101:-1]
    ax = data.ax
    ay = data.ay
    az = data.az
    y = data.y
    p = data.p
    r = data.r
    row = list(ax)+list(ay)+list(az)+list(y)+list(p)+list(r)
    X_test.append(row)

    X_test = np.array(X_test)  
    return X_test

def get_label(json_data):
    df_input = json_normalize(json_data)
    #print(df_input)
    X = get_X_series_concat(df_input)
    #print(model.predict_proba(X))
    return int(model.predict(X))

def processing():
    '''Return a list of json object with 6 attributes: ax,ay,az,y,p,r'''
    global prev
    while(True):
        #print("cool - ")
        try:
            json_data = requests.get(api_url).json()
            #print(len(json_data))
            if(len(json_data)<100):
                continue    
            label =  get_label(json_data)
            client.publish(topic_pub, label)

            if(label==1):
                client.publish("status", "Drop")
                time.sleep(3)
            elif(label==0):
                client.publish("status", "Normal")
            
            
            print('label = ', label)
            time.sleep(0.01)
            #break
        except requests.exceptions.ConnectionError:
            print("Connection refused. Reconnect in 2s")
            time.sleep(1)

    #json_data = requests.post(api_url)
    # json_status = json_data[0]
    # print("Api status: ", json_status)
    # print("cool")
    # print(json_data)

def main():
    init_connection()
    processing()

main()