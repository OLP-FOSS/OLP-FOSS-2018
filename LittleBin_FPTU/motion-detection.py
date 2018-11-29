import time
import json
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split, learning_curve, ShuffleSplit
from sklearn import metrics
from sklearn.naive_bayes import GaussianNB, MultinomialNB
import paho.mqtt.client as mqtt

# df_raw_normal = pd.read_excel('dataRaw.xlsx', 'nomal')
# df_raw_normal['label'] = 0
# df_raw_moveLeftRight = pd.read_excel('dataRaw.xlsx', 'moveLeftRight')
# df_raw_moveLeftRight['label'] = 1
# df_raw_moveUpDown = pd.read_excel('dataRaw.xlsx', 'moveUpDown')
# df_raw_moveUpDown['label'] = 2
# df_raw_goOnAndBack = pd.read_excel('dataRaw.xlsx', 'GoOnAndBack')
# df_raw_goOnAndBack['label'] = 3
df_raw_0 = pd.read_csv('motionData/data_0.csv')
df_raw_1 = pd.read_csv('motionData/data_1.csv')
df_raw_1_1 = pd.read_csv('motionData/data_1_1.csv')

df_raw = pd.concat([df_raw_0, df_raw_1, df_raw_1_1])

data_label = df_raw['3']
df_raw = df_raw.drop(['3'], axis=1)

X_train, X_test, Y_train, Y_test = train_test_split(df_raw, data_label, test_size=0.33, random_state=42)

model = GaussianNB()
# model.fit(X_train, Y_train)
model.fit(df_raw, data_label)
# print(model)

# make predictions
# expected = Y_test.values
# print(expected)

# predicted = model.predict(X_test)
# print(predicted)
# summarize the fit of the model
# print(metrics.classification_report(expected, predicted))

label = 1
data_save = []

def on_message(client, userdata, message):
  msg_payload = json.loads(str(message.payload.decode("utf-8")))
  data_predict = [msg_payload['roll'], msg_payload['pitch'], msg_payload['yaw'], label]
  # print(data_predict)
  # data_save.append(data_predict)
  # print(len(data_save))
  predicted = model.predict([data_predict])
  # print(predicted[0])
  client.publish('predicted', '{"predict": "%d"}' % predicted[0])

client = mqtt.Client('PythonBackend')
client.on_message=on_message
client.connect('192.168.43.128')
client.publish('event', 'hello world from backend')
client.loop_start()
try:
  while True:
    client.subscribe('predict')
    time.sleep(2)
except KeyboardInterrupt:
  client.loop_stop()
  # df_data_save = pd.DataFrame(data_save)
  # df_data_save.to_csv('data_1_1.csv')
  pass

